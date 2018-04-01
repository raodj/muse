#ifndef LOCK_FREE_PQ_H
#define LOCK_FREE_PQ_H

//---------------------------------------------------------------------------
// 
// This queue is based on work by Linden and Jonsson (2013)
//
// Copyright (c) Miami University, Oxford, OHIO.
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
//---------------------------------------------------------------------------

#include <stdlib.h>
#include <inttypes.h>

#define KEY_NULL 0
#define NUM_LEVELS 32
// todo(deperomm): make this dynamic, paper says should = thread count
#define MAX_OFFSET 8
// Key values for first/last dummy nodes
#define KEY_MIN ( 0UL)
#define KEY_MAX (~1UL)

typedef unsigned long pkey_t;
typedef char          *pval_t;

//Node type, the structure for a single node in the queue
struct Node_t {
    pkey_t  key;       // value for computing priority in queue
    int     level;     // level in skip list, starts at 0
    int     inserting; // 1 if this node is actively being inserted, else 0
    pval_t  value;     // pointer to the value of this node
    Node_t  *next[1];  // array - ptrs for "next" node at each skip list level
    // Note: size of next[] is based on the level of the node in the skip list
    // and is calculated at create time for the node, but is at least size 1
};

// From Linden and Jonsson, use a marker on the pointer to indicate if deleted
#define get_marked_ref(_p)      ((Node_t *)(((uintptr_t)(_p)) | 1))
#define get_unmarked_ref(_p)    ((Node_t *)(((uintptr_t)(_p)) & ~1))
#define is_marked_ref(_p)       (((uintptr_t)(_p)) & 1)

/**
 * A lock free, thread safe priority queue based on the skip list data structure
 * 
 * This queue is based on the work of Linden and Jonsson (2013)
 * Paper: http://www.it.uu.se/research/publications/reports/2013-025/
 * Code:  https://github.com/jonatanlinden/PR
 * 
 * This queue allows for concurrent insert and deleteMin operations. It is based
 * on the skip list data structure.
 * 
 * Concurrency and linearizability is achived by differentiating between 
 * logical and physical deletes, combining the "next" pointer with the logical 
 * delete flag for a node, and also utilizing compare and swap (CAS) 
 * instructions on the CPU. The details of which can be read in the above paper
 */
class LockFreePQ {
    
public:
    /**
     * Constructor and destructor, initialize the PQ and finalize it
     */
    LockFreePQ();
    ~LockFreePQ();
    
    /**
     * Inserts a new value into the queue. Duplicate keys will be skipped
     * 
     * If an entry in the queue exists with the same key, the insert will be
     * skipped and the value not added to the queue.
     * 
     * @param key - unsigned int, the value with which priority is calculated
     * @param value - pointer to the value of this entry in the queue
     */
    void insert(pkey_t key, pval_t value);
    
    /**
     * Removes and returns the value in the queue with the lowest key
     * 
     * @return value - pointer to the value of lowest priority node
     */
    pval_t deleteMin();
    
    /**
     * Visualizes the current state of the skip list for debugging
     * 
     * Also validates that skip links are correct for all nodes
     * 
     * prints to std::cout buffer
     */
    void prettyPrint();
    
private:
    
    /**
     * Gets the current predecessors and successors at all skip list levels
     * 
     * This information is used by the insert method to attempt to quickly
     * insert the node in the queue. It's possible these preds and succs have
     * changed after this is called, so must use compare and swap to actually
     * do the insert.
     * 
     * @return del - the last deleted node on the bottom level, could be null
     */
    Node_t* locatePreds(pkey_t k, Node_t ** preds, Node_t ** succs);
    
    /**
     * Restructures higher level pointers following a deletion
     * 
     * A primary optimization of this queue is that this method is only called
     * after a certain number of logically deleted nodes pile up at the front
     * of the queue (specified by the value maxOffset)
     */
    void restructure();
    
    /**
     * Allocates, initializes, and returns new node for insertion into the queue
     * 
     * The returned node does not have any value associated with it, and the
     * node still needs to be linked into the queue before it can be useful.
     * 
     * @return Node_t - the newly allocated queue node
     */
    static Node_t* allocNode();
    
    /**
     * Releases a node from memory
     * 
     * eventually would like this to be handled by a garbage collector
     * 
     * @param n - Node_t, the node being marked as no longer needed by the queue
     */
    static void freeNode(Node_t *n) {
//        delete *n; not necessary because no destructor for Node_t?
        free(n);
    }
    
    // variables and pointers
    int     maxOffset;
    Node_t  *head;
    Node_t  *tail;
    
    // Used for linear conguential generator (LCG) for rand levels of skip list
    static unsigned int  lcg_rand;
};

#endif /* LOCK_FREE_PQ_H */

