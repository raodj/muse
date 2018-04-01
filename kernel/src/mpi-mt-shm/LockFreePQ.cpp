#ifndef LOCK_FREE_PQ_CPP
#define LOCK_FREE_PQ_CPP

#include "mpi-mt-shm/LockFreePQ.h"
#include <stdlib.h> 
#include <iostream>
#include <assert.h>

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

// static LCG rand num generation value
unsigned int LockFreePQ::lcg_rand = rand();

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
    head->level     = NUM_LEVELS;
    tail->level     = NUM_LEVELS;
    head->value     = "HEAD";
    tail->value     = "TAIL";
    
    // At all levels of the skip list, connect head and tail
    for (size_t i = 0; i < NUM_LEVELS; i++) {
        head->next[i] = tail;
    }
    
    maxOffset = MAX_OFFSET; // todo(deperomm): make this cmd line arg
    
    // A new random value used for LCG in fast random num generation
    lcg_rand = lcg_rand * 1103515245 + 12345;
}

LockFreePQ::~LockFreePQ() {
    Node_t *cur, *pred;
    cur = head;
    while (cur != tail) {
        pred = cur;
        cur = get_unmarked_ref(pred->next[0]);
        freeNode(pred);
    }
    free(tail);
}

void
LockFreePQ::insert(pkey_t k, pval_t v) {
    
    assert(KEY_MIN < k && k < KEY_MAX);
    
    Node_t *new_node;
    new_node = allocNode();
    Node_t *del      = NULL;
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    
    new_node->key   = k;
    new_node->value = v;
    
    // Insert at into bottom level
    bool inserted;
    do {
        del = locatePreds(k, preds, succs);
        
        // return if key already exists, i.e., is present in a non-deleted node
        if (succs[0]->key == k 
                && !is_marked_ref(preds[0]->next[0]) 
                && preds[0]->next[0] == succs[0]) {
            new_node->inserting = 0;
            freeNode(new_node);
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
            if (succs[0] != new_node) break;
	    
        } else {
            // Succeeded at this level, move on to next
            i++;
        }
    }
    
    if (new_node) {
        new_node->inserting = 0;
    }   
}

pval_t
LockFreePQ::deleteMin() {
    // initial variables
    pval_t ret = NULL;
    Node_t *pred, *succ, *obsHead = NULL, *newHead = NULL, *cur;
    int offset = 0;
    
    pred = head;
    // first real item in the queue, could be deleted
    obsHead = pred->next[0];
    
    do {
        offset++;
        
        succ = pred->next[0];
        
        // if list is empty, we return null
        if (get_unmarked_ref(succ) == tail) {
            return ret; // null
        }
        
        // don't allow newHead to be past anything currently inserting, we don't
        // want the new front of the queue to be behind something being inserted
        // this is because a node may be inserted in front of (lower than) the
        // node we want to delete (if it came in after we started deleteMin)
        // ex.(deleted* inserting^): 1* 2* 4^ 6* 7 9  return 7 but newHead = 4
        if (newHead == NULL && pred->inserting) newHead = pred;
        
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
        
        // set pred = succ in while loop check so that it gets run even when we
        // continue above
    } while ((pred = get_unmarked_ref(succ)) && is_marked_ref(succ));
    
    assert(!is_marked_ref(pred));
    
    ret = pred->value;
    
    // if we didn't traverse any actively inserting nodes, new head is pred
    if (newHead == NULL) newHead = pred;
    
    // only continue to physical deletion of nodes if we are past our threshold
    if (offset <= maxOffset) return ret;
    
    // We set obsHead = head->next[0]. If those two are still equal, swap
    // obsHead with our new head, thus skipping any node that was before newHead
    // If the two values are no longer equal, another thread probably already
    // did this operation, and we just skip restructuring
    if (__sync_bool_compare_and_swap(&head->next[0], 
            obsHead, get_marked_ref(newHead))) { // (deperomm): can't newHead be non-deleted if it was inserting?
        
        // restructure higher level pointers to account for this delete
        restructure();
        
        // if we reach this point, all nodes between obsHead and newHead are
        // guaranteed not active and safe to delete
        cur = get_unmarked_ref(obsHead);
        while (cur != get_unmarked_ref(newHead)) {
            succ = get_unmarked_ref(cur->next[0]);
            assert(is_marked_ref(cur->next[0])); // make sure marked for delete
            freeNode(cur); // todo(deperomm): Custom garbage collection/recycling
            cur = succ;
        }
    }
    
    return ret;
}

Node_t*
LockFreePQ::locatePreds(pkey_t k, Node_t ** preds, Node_t ** succs) {
    Node_t *pred, *succ, *del = NULL;
    int d, i;
    
    pred = head;
    
    // traverse the skip list top down, getting pred and succ at each level
    i = NUM_LEVELS -1;
    while (i >= 0) {
        // traverse the first node at this level
        succ = pred->next[i];
        d = is_marked_ref(succ);
        succ = get_unmarked_ref(succ);
        assert(succ != NULL);
        
        // traverse all nodes at this level until we find where we belong
        // note that we continue if
        //     1. the current successor's key less than k
        //     2. the next successor node is marked for deletion
        //     3. on bottom level: the current successor was marked for delete
        // the bot level does this b/c at high levels, we continue until the
        // current succ is deleted, at the bottom level cont until succ
        while (succ->key < k || is_marked_ref(succ->next[0])
               || ((i == 0) && d)) {
            // on the bottom level, record the last seen deleted node
            if (i == 0 && d)
                del = succ;
            pred = succ;
            succ = pred->next[i];
            d = is_marked_ref(succ);
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
        h = head->next[i];
        cur = pred->next[i];
        
        // if the first value at this level isn't deleted, just continue
        if (!is_marked_ref(h->next[0])) {
            i--;
            continue;
        }
        
        // else, traverse this level until a non-deleted node is found 
        // which could be the tail
        // It's also important to note that pred is always a deleted node at
        // this point, so when we move down levels in subsequent while loops,
        // we continue traversing where pred left off
        while (is_marked_ref(pred->next[0])) {
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
LockFreePQ::prettyPrint() {
    Node_t *cur = head, *last;
    
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
            cur = cur->next[0];
            if (cur->level >= i) {
                std::cout << "|"; // node reaches up to this level
                assert(last->next[i] == cur); // make sure link is correct
                last = cur;
            } else {
                std::cout << "-"; // node is skipped at this level
            }
        } while (cur != tail);
        std::cout << std::endl;
        
        i--;
    }
    
}

int main() {
    LockFreePQ pq = LockFreePQ();
    
    pq.insert(2, "two");
    pq.insert(5, "five");
    pq.insert(13, "thirteen");
    pq.insert(1, "one");
    pq.insert(9, "nine");
    pq.insert(12, "twelve");
    pq.insert(7, "seven");
    pq.insert(10, "ten");
    pq.insert(4, "four");
    pq.prettyPrint();
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    pq.insert(11, "eleven");
    pq.insert(15, "fifteen");
    pq.insert(14, "fourteen");
    pq.insert(6, "six");
    pq.insert(8, "eight");
    pq.insert(3, "three");
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
    std::cout << "out: " << pq.deleteMin() << std::endl;
}

#endif