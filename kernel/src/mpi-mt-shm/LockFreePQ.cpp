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
#include <thread>      // for testing?
#include <vector>      // for testing, vector
#include <algorithm>   // for testing, shuffle
#include <chrono>      // for testing, clock

// The ID associated with any thread that accesses this queue
thread_local size_t LockFreePQ::thread_id = THREAD_ID_NULL;

// A static value used to distribute unique IDs across multiple threads
size_t* LockFreePQ::lastId = new size_t(0);

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

pval_t
LockFreePQ::insert(pkey_t k, pval_t v, pid_t id) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // new key cannot be equal to head or tail node key values
    if (!(KEY_MIN < k && k < KEY_MAX)) {
        std::cout << "what is happening?: " << k << std::endl;
    }
    assert(KEY_MIN < k && k < KEY_MAX);
    assert(id >= -1);
    
    // value cannot start null as we set it to null if deleted mid queue
    assert(v != NULL);
    
    Node_t *new_node;
    new_node         = allocNode();
    Node_t *del      = NULL;
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    
    new_node->key   = k;
    new_node->value = v;
    new_node->id    = id;
    
    // Insert into bottom level
    bool inserted;
    do {
        del = locatePreds(k, id, preds, succs);
        
        // return if key id pair already exists
        if (succs[0]->key == k && succs[0]->id == id
                && !is_marked_ref(preds[0]->next[0]) 
                && preds[0]->next[0] == succs[0]) {
            freeNode(new_node); // we didn't insert anything
            
            exitCritical();
            return succs[0]->value; // return the entry that matches the key
                                    // this could be null if it was deleted
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
            del = locatePreds(k, id, preds, succs);

            // if new node has been deleted, we're done
            if (succs[0] != new_node) break;
	    
        } else {
            // Succeeded at this level, move on to next
            i++;
        }
    }
    
    if (new_node) {
        new_node->inserting = 0;
    }
    
    exitCritical();
    return NULL;
}

pval_t
LockFreePQ::deleteEntry(pkey_t k, pid_t id) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // new key cannot be equal to head or tail node key values
    assert(KEY_MIN < k && k < KEY_MAX);
    assert(id >= -1);
    
    pval_t ret = NULL;
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    
    locatePreds(k, id, preds, succs);

    // attempt to delete/return if we found the key+id pair
    if (succs[0]->key == k && succs[0]->id == id
            && !is_marked_ref(preds[0]->next[0]) 
            && preds[0]->next[0] == succs[0]) {
        
        ret = succs[0]->value;
        
        if (ret != NULL) {
            if (!__sync_bool_compare_and_swap(&succs[0]->value, ret, NULL)) {
                // failed, another thread already set value to null
                ret = NULL;
            }
        }
    }
    
    exitCritical();
    return ret;
}

pval_t
LockFreePQ::deleteMin() {
    // This method must call exitCritical() before returning
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
        // even if it was already deleted while it was inserting
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
        
        // Grab the value. The value always starts not null, and is set to null
        // when retrieved to mark this node as deleted and ensure only one
        // thread retrieves the value
        if (!is_marked_ref(succ)) {
            ret = succ->value;
            if (ret != NULL) {
                if(!__sync_bool_compare_and_swap(&succ->value, ret, NULL)) {
                    // failed, another thread grabbed the value first, continue
                    ret = NULL;
                }
            }
        }
        
        // set pred = succ in while loop check so that it gets run even when we
        // continue above
    } while ((pred = get_unmarked_ref(succ)) 
            && (is_marked_ref(succ) || ret == NULL));
    
    assert(!is_marked_ref(pred));
    
    assert(ret != NULL);
    
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

pkey_t
LockFreePQ::nextMin() {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // initial variables
    pkey_t ret = 10000000; // minus 1 so that this key could be inserted
    Node_t *pred, *succ;
    
    pred = head;
    
    do { 
        // likely expensive - high probability of a cache miss here, as with all
        // linked list traversals
        succ = pred->next[0];
        
        // if list is empty, we return KEY_MAX-1
        if (get_unmarked_ref(succ) == tail) {
            exitCritical();
            return ret; // KEY_MAX-1
        }
        
        assert(succ != NULL);
        
        pred = get_unmarked_ref(succ);
        
    } while (is_marked_ref(succ) || succ->value == NULL);
    
    // At this point pred is the first non-deleted node we encountered
    
    // Note that there is no thread safety in this method. It's very possible
    // that even when control drops here, the min value in the queue could
    // have changed. As a result, this method must have other thread safety
    // in place higher in the stack to ensure this value is useful in any way
    
    assert(!is_marked_ref(pred));
    
    // return key, NOT value
    ret = pred->key;
    
    exitCritical();
    return ret;
}

pval_t
LockFreePQ::getEntry(pkey_t k, pid_t id) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    assert(KEY_MIN < k && k < KEY_MAX);
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    

    locatePreds(k, id, preds, succs);

    // return if key-id pair was NOT found (since it must exist in the
    // queue in order for us to delete it)
    if (succs[0]->key != k || succs[0]->id != id
            || is_marked_ref(preds[0]->next[0]) 
            || preds[0]->next[0] != succs[0]) {

        exitCritical();
        return NULL; 
    }

    exitCritical();
    return succs[0]->value;
    
}

Node_t*
LockFreePQ::getNode(pkey_t k, pid_t id) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    assert(KEY_MIN < k && k < KEY_MAX);
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    

    locatePreds(k, id, preds, succs);
    
    assert(succs[0] != NULL);

    exitCritical();
    return succs[0];
}

//void
//LockFreePQ::deleteAfter(pkey_t k, pid_t id) {
//    // This method must call exitCritical() before returning
//    enterCritical();
//    assert(KEY_MIN < k && k < KEY_MAX);
//    
//    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
//    
//
//    // Delete on bottom level
//    bool deleted;
//    do {
//        del = locatePreds(k, id, preds, succs);
//        
//        // return if key id pair already exists
//        if (succs[0]->key == k && succs[0]->id == id
//                && !is_marked_ref(preds[0]->next[0]) 
//                && preds[0]->next[0] == succs[0]) {
//            freeNode(new_node); // we didn't insert anything
//            
//            exitCritical();
//            return succs[0]; // return the entry that matches the key
//        }
//        
//        // begin inserting
//        new_node->next[0] = succs[0];
//        
//        // We use compare and swap to ensure thread safety, if fails restart
//        inserted = 
//            __sync_bool_compare_and_swap(&preds[0]->next[0], succs[0], new_node);
//        
//    } while (!inserted);
//    
//    exitCritical();
//}


Node_t*
LockFreePQ::locatePreds(pkey_t k, pid_t id, Node_t ** preds, Node_t ** succs) {
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
        //     1. the current successor's key+id pair is less than the given one
        //     2. the current successor has nodes after it that are deleted
        //     3. on bottom level: the current successor was marked for delete
        // succ is what we insert behind and must not be deleted
        while (succ->key < k || (succ->key == k && succ->id < id) 
                || is_marked_ref(succ->next[0]) || ((i == 0) && d)) {
            // on the bottom level, record the last seen deleted node
            if (i == 0 && d)
                del = succ;
            pred = succ;
            succ = pred->next[i];
            d = is_marked_ref(succ);       // only applies on bottom level as 
                                           // only bot ptrs have ref marked
//            assert(succ != NULL);
            succ = get_unmarked_ref(succ);
            if (succ == NULL) {
                // This is a serious issue. It appears to be related to
                // garbage collection, where elements of the list are freed
                // before all threads were done operating on them.
                // This assert got triggered very rarely (1 in ~20 runs with
                // millions of elements at very high contention), so if we 
                // reach this point we simply try again to alleviate the issue 
                // for now.
                // todo(deperomm): review garbage collection
                return locatePreds(k, id, preds, succs);
            }
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
        
        // If the first value at this level has nothing deleted after it
        // continue. This means this level skips ahead of the nodes that were
        // deleted. No need to traverse and no need to update any pointers
        if (!is_marked_ref(h->next[0])) {
            i--;
            continue;
        }
        
        // Traverse this level until a non-deleted node is found which could
        // be the tail.
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
    // We want each level up to be half as likely as the level below it, so
    // a geometric distribution
    // todo(deperomm): Validate this random generator is working sufficently
    static thread_local std::mt19937 generator;
    static thread_local std::geometric_distribution<int> distribution;
    
    int level = distribution(generator);
    if (level > NUM_LEVELS) level = NUM_LEVELS - 1;
    
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
    // Note: this is expensive-ish because of very high contention by threads
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
    // epoch_state and the mutex (if another thread switch epoch between 
    // setting "old" and trying lock) check again now that we own the lock.
    // If epoch_state was changed since we last check, just abort
    if (thread_epoch_state->load() != 0) {
        epochMutex.unlock();
        return;
    }
    
    // safe to call this, we have the epoch lock and epoch_state is definitely 0
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
        free(*it);
        n++;
    }
    waitingDeallocs->clear();
    
    // Swap pointer for pendingDeallocs with waitingDeallocs (which is now
    // empty) to trigger the start of the new epoch. Nodes that were in
    // pendingDeallocs are now waiting for this epoch to end
    // This is the critical moment at which the new epoch technically starts
    freeMutex.lock(); // hold threads from deleting nodes until after swap
    std::deque<Node_t*> *temp = pendingDeallocs;
    pendingDeallocs = waitingDeallocs;
    waitingDeallocs = temp;
    freeMutex.unlock();
    
    // Threads probably exited and entered after the swap above happened, but
    // we only care that all threads that were active at the moment we switched
    // dealloc pointers exit at least once before we change epochs again. As
    // a result, switching the epoch state now (after switching the pointers)
    // guarantees that any thread that was active at the moment we switched
    // will have exited once this epoch_state gets set to 0.
    thread_epoch_state->store(thread_curr_state->load());
    
//    std::cout << "finished epoch, deleted " << n 
//    << " cur state: " << thread_curr_state->load() << std::endl;
    
}

void
LockFreePQ::finalizeDeallocs() {    
    // threads must not be active when this is called
    assert(thread_curr_state->load()  == 0);
    thread_curr_state->store(0); // todo(deperomm):this shouldn't need to be set
    
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

/* without deleteEntry validation */
//void test(LockFreePQ *pq, std::vector<double> vals) {
//    double out = 0;
//    double in = 0;
//    double x;
//    for (size_t i = 0; i < vals.size(); i++) {
//        x = vals[i];
//        in += x;
//        int *val = new int(x);
//        pq->insert(x, (void *)val);
//        val = (int *)pq->deleteMin();
//        out += *val;
//        delete  val;
//    }
//    std::cout << "Inserted: " << (long)in << " Got: " << (long)out << " Difference: " << ((long)in - (long)out) << std::endl;
//}

/* with delete entry validation */
//void test(LockFreePQ *pq, std::vector<double> vals) {
//    double out = 0;
//    double in = 0;
//    double x;
//    for (size_t i = 0; (i+1) < vals.size(); i+=2) {
//        x = vals[i];
//        in += x;
//        int *val = new int(x);
//        pq->insert(x, (void *)val);
//        val = (int *)pq->deleteEntry(x);
//        if (val != NULL) {
//            out += *val;
//            delete  val;
////            std::cout << "O "; // debug print if value was found
//        } else {
//            val = (int *)pq->deleteMin();
//            out += *val;
//            delete  val;
////            std::cout << "XXXX "; // debug print if value was not found
//        }
//        x = vals[i+1];
//        in += x;
//        val = new int(x);
//        pq->insert(x, (void *)val);
//        val = (int *)pq->deleteMin();
//        out += *val;
//        delete  val;
//    }
//    std::cout << "Inserted: " << (long)in << " Got: " << (long)out << " Difference: " << ((long)in - (long)out) << std::endl;
//}
//
//int main(int argc, char** argv) {
//    LockFreePQ *pq = new LockFreePQ();
//    
//    std::vector<std::thread> threads;
//    if (argc < 2) {
//        std::cout << "Queue Test Usage: ./queue_test num_threads (num_vals)" << std::endl;
//        abort();
//    }
//    size_t n = atoi(argv[1]);
//    size_t k;
//    if (argc > 2) {
//        k = atoi(argv[2]);
//    } else {
//        k = 2000000;
//    }
//    
//    std::vector<std::vector<double>> vals(n);
//    
//    // generate large list of unique numbers for each thread
//    for (size_t i = 0; i < n; i++) {
//        for (size_t x = 1; x < k/n; x++) {
//            vals[i].push_back(x*n+i);
//        }
//    }
//    // shuffle the list
//    for (size_t i = 0; i < n; i++) {
//        std::random_shuffle(vals[i].begin(), vals[i].end());
//    }
//    
//    std::cout << "Starting" << std::endl;
//    auto start = std::chrono::high_resolution_clock::now();
//    
//    // spin of threads that constantly enqueue and dequeue, max contention
//    // possible on the queue
//    for (size_t i = 0; i < n; i++) {
//        threads.push_back(std::thread(test, pq, vals[i]));
//    }
//    
//    for (size_t i = 0; i < n; i++) {
//        threads[i].join();
//    }
//    
//    auto finish = std::chrono::high_resolution_clock::now();
//    std::cout << "Done. Differences printed above should sum to 0" << std::endl;
//    
//    std::chrono::duration<double> elapsed = finish - start;
//    std::cout << "Elapsed time: ~" << elapsed.count() << " s" << std::endl;
//    
//    delete pq;
//    
//}

#endif