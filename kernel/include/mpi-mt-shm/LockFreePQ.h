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
#include <inttypes.h>  // additional int types, specifically uintptr_t
#include <climits>     // numeric type limits i.e. ULONG_MAX
#include <mutex>
#include <deque>
#include <atomic>

#define KEY_NULL 0
#define NUM_LEVELS 32
// todo(deperomm): make this dynamic, paper says should = thread count
#define MAX_OFFSET 8
// Key values for first/last dummy nodes
#define KEY_MIN (0)
#define KEY_MAX (ULONG_MAX)
// Used to initially set thread_id to an invalid value
#define THREAD_ID_NULL (UINT_MAX)

typedef unsigned long pkey_t;
typedef void          *pval_t;

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
     * 
     * Responsible for setting initial state and clearing memory
     */
    LockFreePQ();
    
    /**
     * Cleans up the queue
     * 
     * This operation is NOT thread safe, and should only be called after 
     * all operations are done with the queue.
     */
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
     * ** Note: This method is not thread safe
     * 
     * prints to std::cout buffer
     */
    void print();
    
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
     * Marks a node as ready for deletion and deletes nodes when safe to do so
     * 
     * Because threads could still be operating on memory that is ready for
     * deletion, must wait until all threads have left critical sections before
     * physically deleting the memory.
     * 
     * Nodes that are freed should be totally disconnected from the queue,
     * meaning any threads that newly enters a critical section will not ever
     * reach the to-be-freed node. 
     * 
     * @param n - Node_t, the node being marked as no longer needed by the queue
     */
    void freeNode(Node_t *n);
    
    /**
     * Alerts the garbage collector that a thread has entered a critical section
     * 
     * When called for first time on a thread, also initializes garabage
     * collection for that thread
     */
    void enterCritical();
    
    /**
     * Alerts garbage collection that this thread has left a critical section
     * 
     * Once all threads have left their critical sections, we move forward
     * one epoch. Nodes that have waited at least a full epoch since being
     * released are guarenteed to no longer be accessed by any thread
     */
    void exitCritical();
    
    /**
     * Goes through the process of changing epoch and garbage collecting
     * 
     * This thread should only be called by a single thread exclusively
     * and therefore must be surrounded by a lock
     */
    void garbageCollectAndChangeEpoch();
    
    /**
     * When a new thread accesses the queue, it is assigned an id for garbage
     * collection purposes
     * 
     * thread_id is always a unique power of two, as it is used for bitwise ops
     * to toggle overall thread state
     * 
     * For example, the 3rd thread in the system would get thread_id = 0b100
     */
    thread_local static size_t thread_id;
    size_t *lastId = new size_t(0);        // used to distribute IDs to threads
    
    /**
     * bitmap which represents which threads are acitvely in critical sections
     * 
     * Each bit represents a thread actively in a critical section, when a 
     * thread leaves its critical section it flips its bit back to 0
     * 
     * For example, if state = 0110 then two threads are actively in crit secs
     */
    std::atomic_uint *thread_curr_state = new std::atomic_uint;
    
    /**
     * bitmap which represents threads that were active when epoch last switched
     * 
     * This is specifically set after the moment the epoch switched to ensure
     * that even if new threads entered and previously active threads exited
     * since the moment we switched, we guarantee that when this is zero, all
     * threads that were active at the moment we switched have since exited.
     */
    std::atomic_uint *thread_epoch_state = new std::atomic_uint;
    
    /**
     * For garbage collection, any new node that is being marked for deletion is
     * added to this vector. These nodes are very likely not safe to delete
     * as other threads could still use them.
     */
    std::deque<Node_t*> *pendingDeallocs = new std::deque<Node_t*>;
    
    /**
     * For garbage collection, when we switch epochs (all threads since last
     * epoch change have left their critical sections), this vector is safe
     * to free (since it has been at least a full epoch since this vector was
     * populated, meaning every node was released during the previous epoch). 
     * 
     * Every epoch, this queue is processed to free memory and then populated
     * with the contents of pendingDeallocs.
     */
    std::deque<Node_t*> *waitingDeallocs = new std::deque<Node_t*>;
    
    /**
     * Finalizes the entire garbage collection system for queue tear down
     * 
     * Clears all pending and waiting deallocations that were removed from the 
     * queue but not physically deleted yet
     * 
     * This should only be called when tearing down the queue and is NOT
     * thread safe.
     */
    void finalizeDeallocs();
    
    /**
     * a mutex to ensure safe insertion into pendingDeallocs queue
     */
    std::mutex freeMutex;
    
    /**
     * a mutex to ensure only one thread changes epoch at a time
     */
    std::mutex epochMutex;
    
    // variables and pointers
    int     maxOffset;
    Node_t  *head;
    Node_t  *tail;
    
    // Used for linear conguential generator (LCG) for rand levels of skip list
    // todo(deperomm): better rand value generator
    thread_local static unsigned int lcg_rand;
};

//g++ -std=c++11 -g -Wall -Wextra -I ../../include -pthread -fsanitize=thread -fPIE -pie LockFreePQ.cpp -o test


#endif /* LOCK_FREE_PQ_H */

