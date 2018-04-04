#ifndef LOCK_FREE_PQ_CPP
#define LOCK_FREE_PQ_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OH.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//--------------------------------------------------------------------------

#include "mpi-mt-shm/LockFreePQ.h"

#include <stdlib.h> 
#include <iostream>
#include <assert.h>
#include <math.h>      // pow()
#include <thread>
#include <vector>

// The ID associated with any thread that access this queue
thread_local size_t LockFreePQ::thread_id = THREAD_ID_NULL;

// static LCG rand num generation value
// todo(deperomm): better rand value generator
thread_local unsigned int LockFreePQ::lcg_rand = rand();

LockFreePQ::LockFreePQ() {
    
    // Dynamically adjust the size of *head and *tail so they can hold 
    // pointers to all levels in head.next[]
    // We use (NUM_LEVELS-1) because sizeof *head/tail already includes next[1]
    head = (Node_t *)calloc(1, sizeof *head + (NUM_LEVELS-1)*sizeof(Node_t *));
    tail = (Node_t *)calloc(1, sizeof *tail + (NUM_LEVELS-1)*sizeof(Node_t *));
    
    // Set initial values
    head->inserting = 0;
    tail->inserting = 0;
    head->key       = KEY_MIN;
    tail->key       = KEY_MAX;
    head->level     = NUM_LEVELS - 1;
    tail->level     = NUM_LEVELS - 1;
    
    // At all levels of the skip list, connect head and tail
    for (size_t i = 0; i < NUM_LEVELS; i++) {
        head->next[i] = tail;
    }
    
    maxOffset = MAX_OFFSET; // todo(deperomm): make this cmd line arg
    
    // A new random value used for LCG in fast random num generation
    lcg_rand = lcg_rand * 1103515245 + 12345;
}

LockFreePQ::~LockFreePQ() {
    
    // Clean up current elements of the queue
    Node_t *cur, *pred;
    cur = head;
    while (cur != tail) {
        pred = cur;
        cur = get_unmarked_ref(pred->next[0]);
        freeNode(pred);
    }
    freeNode(tail);
    
    // Clean up the garbage collection system
    finalizeDeallocs();
}

// Note: This method must call exitCritical() before returning
void
LockFreePQ::insert(pkey_t k, pval_t v) {
    
    enterCritical();
    
    // new key cannot be equal to head or tail node key values
    assert(KEY_MIN < k && k < KEY_MAX);
    
    Node_t *new_node;
    new_node         = allocNode();
    Node_t *del      = NULL;
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    
    new_node->key   = k;
    new_node->value = v;
    
    // Insert into bottom level
    bool inserted;
    do {
        del = locatePreds(k, preds, succs);
        
        // return if key already exists, i.e., is present in a non-deleted node
        if (succs[0]->key == k 
                && !is_marked_ref(preds[0]->next[0]) 
                && preds[0]->next[0] == succs[0]) {
            std::cout << "WARNING: Duplicate entry not added." << std::endl;
            freeNode(new_node);
            
            exitCritical();
            return;
        }
        
        // begin inserting
        new_node->next[0] = succs[0];
        
        // We use compare and swap to ensure thread safety, if fails restart
        inserted = 
            __sync_bool_compare_and_swap(&preds[0]->next[0], succs[0], new_node);
        
    } while (!inserted);
    
    // Insert at all higher levels
    int i = 1;
    while (i <= new_node->level) {
        // If at this stage we find a successor marked for deletion, we can
        // stop inserting at higher levels and just continue
        if (is_marked_ref(new_node->next[0]) ||
            is_marked_ref(succs[i]->next[0]) ||
            del == succs[i])
            break;

        // begin inserting
        new_node->next[i] = succs[i];
        if (!__sync_bool_compare_and_swap(&preds[i]->next[i], succs[i], new_node))
        {
            // failed b/c competing insert or restructure, try this level again
            del = locatePreds(k, preds, succs);

            // if new node has been deleted, we're done
            if (succs[0] != new_node) break; // This is why cannot have dup keys
	    
        } else {
            // Succeeded at this level, move on to next
            i++;
        }
    }
    
    if (new_node) {
        new_node->inserting = 0;
    }
    
    exitCritical();
}

// Note: This method must call exitCritical() before returning
pval_t
LockFreePQ::deleteMin() {
    
    enterCritical();
    
    // initial variables
    pval_t ret = NULL;
    Node_t *pred, *succ, *obsHead = NULL, *newHead = NULL, *cur;
    int offset = 0;
    
    pred = head;
    // first real item in the queue, could be deleted
    obsHead = head->next[0];
    
    do {
        offset++;
        
        // likely expensive - high probability of a cache miss here, as with all
        // linked list traversals
        succ = pred->next[0];
        
        // if list is empty, we return null
        if (get_unmarked_ref(succ) == tail) {
            exitCritical();
            return ret; // null
        }
        
        // don't allow newHead to be past anything currently inserting, we don't
        // want the new front of the queue to be behind something being inserted
        // this is because a node may be inserted in front of (lower than) the
        // node we want to delete (if it came in after we started deleteMin)
        // ex.(deleted* inserting^): 1* 2* 4^ 6* 7 9  return 7 but newHead = 4
        if (newHead == NULL && pred->inserting) {
            newHead = pred;
        }
        
        // if the next node is already marked for deletion, just continue
        if (is_marked_ref(succ)) continue;
        
        // Actual deletion happens here...
        // FAO (fetch and or) instruction marks the pointer pred->next[0] as
        // deleted (thus logically deleting succ) and returns the original value
        // - If the new pointer succ is marked after this operation, then succ
        //   was marked first by another thread since the above if statment
        // - If the new pointer succ is NOT marked, that means we successfully
        //   marked it for deletion before another thread and we exit the loop
        // the above logic happens in the while loop condition
        succ = __sync_fetch_and_or(&pred->next[0], 1);
        
        assert(succ != NULL);
        
        // set pred = succ in while loop check so that it gets run even when we
        // continue above
    } while ((pred = get_unmarked_ref(succ)) && is_marked_ref(succ));
    
    assert(!is_marked_ref(pred));
    
    ret = pred->value;
    
    // if we didn't traverse any actively inserting nodes, new head is pred
    if (newHead == NULL) newHead = pred;
    
    // only continue to physical deletion of nodes if we are past our threshold
    if (offset <= maxOffset) {
        exitCritical();
        return ret;
    }
    
    // We set obsHead = head->next[0]. If those two are still equal, swap
    // obsHead with our new head, thus skipping any node that was before newHead
    // If the two values are no longer equal, another thread probably already
    // did this operation, and we just skip restructuring
    if (__sync_bool_compare_and_swap(&head->next[0], 
            obsHead, get_marked_ref(newHead))) {
        
        // restructure higher level pointers to account for this delete
        restructure();
        
        // if we reach this point, all nodes between obsHead and newHead are
        // guaranteed not accessible by the queue and safe to mark for delete
        cur = get_unmarked_ref(obsHead);
        while (cur != get_unmarked_ref(newHead)) {
            succ = get_unmarked_ref(cur->next[0]);
            assert(is_marked_ref(cur->next[0]));
            freeNode(cur);
            cur = succ;
        }
    }
    
    exitCritical();
    return ret;
}

Node_t*
LockFreePQ::locatePreds(pkey_t k, Node_t ** preds, Node_t ** succs) {
    Node_t *pred, *succ, *del = NULL;
    int d = 0, i;
    
    pred = head;
    
    // traverse the skip list top down, getting pred and succ at each level
    i = NUM_LEVELS -1;
    while (i >= 0) {
        // traverse the first node at this level
        succ = pred->next[i];
        d = is_marked_ref(succ); // this only applies on bottom level as 
                                 // only bot level next ptrs have ref marked
        succ = get_unmarked_ref(succ);
        assert(succ != NULL);
        
        // traverse all nodes at this level until we find where we belong
        // note that we continue if
        //     1. the current successor's key less than k
        //     2. the current successor has nodes after it that are deleted
        //     3. on bottom level: the current successor was marked for delete
        while (succ->key < k || is_marked_ref(succ->next[0])
               || ((i == 0) && d)) {
            // on the bottom level, record the last seen deleted node
            if (i == 0 && d)
                del = succ;
            pred = succ;
            succ = pred->next[i];
            d = is_marked_ref(succ);       // only applies on bottom level as 
                                           // only bot ptrs have ref marked
            succ = get_unmarked_ref(succ);
            assert(succ != NULL);
        }
        preds[i] = pred;
        succs[i] = succ;
        i--;
    }
    return del;
}

void
LockFreePQ::restructure() {
    Node_t *pred, *cur, *h;
    int i = NUM_LEVELS - 1;
    
    pred = head;
    
    while (i > 0) { // don't restructure on the bottom level
        h   = head->next[i];
        cur = pred->next[i];
        
        // if the first value at this level has nothing deleted in front of it
        // continue
        if (!is_marked_ref(h->next[0])) {
            i--;
            continue;
        }
        
        // else, traverse this level until a non-deleted node is found 
        // which could be the tail
        // It's also important to note that pred is always a deleted node at
        // this point, so when we move down levels in subsequent while loops,
        // we continue traversing where pred left off
        while (is_marked_ref(cur->next[0])) {
            pred = cur;
            cur = pred->next[i];
        }
        
        // move the head pointer to the first non-deleted value we saw
        // ensure no other thread did work while we calculated the swap by
        // making sure head->next[i] still equals h before making the swap
        if (__sync_bool_compare_and_swap(&head->next[i],h,cur))
            i--;
    }
}

Node_t*
LockFreePQ::allocNode() {
    Node_t *new_node;
    
    // In skip lists, the level of a node is randomly decided when created
    // Calculate node level via LCG rand num and count num of seqential zeros
    // since prob each bit = 0 is .5, length of zeros is geometric distribution
    // todo(deperomm): better rand value generator
    unsigned int r = lcg_rand;
    lcg_rand = lcg_rand * 1103515245 + 12345;
    r &= (1u << (NUM_LEVELS - 1)) - 1;
    int level = __builtin_ctz(r); // count num sequential zeros
    
    assert(0 <= level && level < NUM_LEVELS);
    
    // Allocate the new node w/ enough space to hold pointers to (level) ptrs
    // sizeof *new_node has enough space for bot level, then + space for others
    // note bottom level is 0, so any higher level adds space to the node
    // todo(deperomm): Make this come from a garbage collector/recycler
    new_node = (Node_t *)calloc(1, sizeof *new_node + (level)*sizeof(Node_t *));
    
    new_node->level     = level;
    new_node->inserting = 1; // nodes always start off as being inserted

    return new_node;
}

void 
LockFreePQ::freeNode(Node_t *n) {
    std::lock_guard<std::mutex> guard(freeMutex);
    pendingDeallocs->push_back(n);
}

void
LockFreePQ::enterCritical() {
    // Initialize thread id if neccessary
    if (thread_id == THREAD_ID_NULL) {
        // This is a new thread entering a critical section for the first time
        
        // Give this thread a thread_id
        size_t candidateId = *lastId;
        while (!__sync_bool_compare_and_swap(lastId, candidateId, candidateId+1)){
            candidateId = *lastId;
        }
        // at this point, we guarantee candidateId is unique to this thread
        thread_id = pow(2, candidateId);
        std::cout << "New Thread - log2(ID) = " << candidateId << std::endl;
    }
    
    assert(thread_id != THREAD_ID_NULL);
    
    // Mark this thread as in a critical section
    // Note: this is expensive because of very high contention by threads
    thread_curr_state->fetch_or(thread_id);
    
}

void
LockFreePQ::exitCritical() {
    
    // Mark this thread as NOT in a critical section
    thread_curr_state->fetch_and(~thread_id);
    
    // Mark the current epoch that this thread is no longer in a crit section
    size_t old = thread_epoch_state->fetch_and(~thread_id);
    
    // If epoch_state has been zero'd out, try to switch epochs, else return
    // We want minimal contention on the atomic state variables, which is why
    // we use the old value instead of checking the value of epoch_state again
    if (old != 0) {
        return;
    }
    
    // If we get the epochMutex, we are the thread changing epoch this time
    // Else just let the other thread take care of it
    if (!epochMutex.try_lock()) {
        return;
    }
    
    // we have the lock
    
    // Because another thread could have changed epochs since we last checked
    // epoch_state, check again now that we officially have the lock
    // If epoch_state was changed since we last check, just abort
    if (thread_epoch_state->load() != 0) {
        epochMutex.unlock();
        return;
    }
    
    // safe to call this, we have the epoch lock
    garbageCollectAndChangeEpoch();
    
    epochMutex.unlock();
}

void
LockFreePQ::garbageCollectAndChangeEpoch() {
    
    assert(!epochMutex.try_lock()); // must be the only thread to run this method
    assert(thread_epoch_state->load() == 0); // must be ready for new epoch
    
    int n = 0; // debug
    
    // First, garbage collect the waiting nodes
    for (auto it = waitingDeallocs->begin(); it != waitingDeallocs->end(); it++) {
        delete *it;
        n++;
    }
    waitingDeallocs->clear();
    
    // Swap pointer for pendingDeallocs with waitingDeallocs (which is now
    // empty) to trigger the start of the new epoch. Nodes that were in
    // pendingDeallocs are now waiting for this epoch to end
    // This is the critical moment at which the new epoch technically starts
    std::deque<Node_t*> *temp = pendingDeallocs;
    pendingDeallocs = waitingDeallocs;
    waitingDeallocs = temp;
    
    // Threads probably exited and entered after the swap above happened, but
    // we only care that all threads that were active at the moment we switched
    // dealloc pointers exit at least once before we change epochs again. As
    // a result, switching the epoch state now (after switching the pointers)
    // guarantees that any thread that was active at the moment we switched
    // will have exited once this epoch_state gets set to 0.
    thread_epoch_state->store(thread_curr_state->load());
    
//     std::cout << "finished epoch, deleted " << n << " cur state: " << thread_curr_state->load() << std::endl;
    
}

void
LockFreePQ::finalizeDeallocs() {
    // threads must not be active when this is called
    assert(thread_curr_state->load()  == 0);
    assert(thread_epoch_state->load() == 0);
    
    // with no active threads, switching epochs twice will clear and garbage
    // collect all dealloc queues
    epochMutex.lock(); // prevents assert from throwing, serves no other purpose
    garbageCollectAndChangeEpoch();
    garbageCollectAndChangeEpoch();
    epochMutex.unlock();
    
    delete thread_curr_state;
    delete thread_epoch_state;
    delete pendingDeallocs;
    delete waitingDeallocs;
    delete lastId;
    
}

void
LockFreePQ::print() {
    Node_t *cur = head, *last;
    bool del;
    
    int i = NUM_LEVELS - 1;
    
    // traverse down until we find the first relevant level
    while (cur->next[i] == tail && i >= 0) {
        i--;
    }
    if (i < 0) {
        std::cout << "Queue is Empty" << std::endl;
        return;
    }
    // we found the top used level, now for each level print all the connections
    while (i >= 0) {
        cur = head;
        last = head;
        std::cout << "|"; // for head entry
        
        do {
            del = is_marked_ref(cur->next[0]);
            cur = get_unmarked_ref(cur->next[0]);
            if (cur->level >= i) {
                if (del) {
                    std::cout << "*"; // this is a deleted node on bot level
                } else {
                    std::cout << "|"; // node reaches up to this level
                    assert(last->next[i] == cur); // make sure link is correct
                }
                last = cur;
            } else {
                std::cout << "-"; // node is skipped at this level
            }
        } while (cur != tail);
        std::cout << std::endl;
        
        i--;
    }
    
}

void test(LockFreePQ *pq, int n, int id) {
    long out = 0;
    long in = 0;
    unsigned long temp;
    for (int i = 100; i < (10000000/n); i++) {
        temp = i*n+id;
        in += temp;
        int *val = new int;
        *val = temp;
        pq->insert(temp, (void *)val);
        val = (int *)pq->deleteMin();
        out += *val;
        delete  val;
    }
    std::cout << "Inserted: " << in << " Got: " << out << std::endl;
}

int main() {
    LockFreePQ *pq = new LockFreePQ();
    
    std::vector<std::thread> threads;
    
    int n = 8;
    
    unsigned long temp;
    long in = 0;
    for (int i = 1; i < 100; i++) {
        temp = i;
        in += temp;
        int *val = new int;
        *val = temp;
        pq->insert(temp, (void *)val);
    }
    std::cout << "init total: " << in << std::endl;
    
    
    
    for (int i = 0; i < n; i++) {
        threads.push_back(std::thread(test, pq, n, i));
    }
    
    for (int i = 0; i < n; i++) {
        threads[i].join();
    }
    
    delete pq;
    
}

#endif