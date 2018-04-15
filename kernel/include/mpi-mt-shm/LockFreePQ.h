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

// debug build command
//g++ -std=c++11 -O3 -Wall -Wextra -I ../../include -pthread -fPIE -pie LockFreePQ.cpp -o test


#include <stdlib.h>
#include <inttypes.h>  // additional int types, specifically uintptr_t
#include <climits>     // numeric type limits
#include <mutex>
#include <deque>
#include <atomic>
#include <random>

#define KEY_NULL 0
#define NUM_LEVELS 32
// todo(deperomm): make this dynamic, must be >= number of threads
#define MAX_OFFSET 8
// Key values for first/last dummy nodes
#define KEY_MIN (0)
#define KEY_MAX std::numeric_limits<double>::max()
// Used to initially set thread_id to an invalid value
#define THREAD_ID_NULL (UINT_MAX)

// todo(deperomm): Replace with c++ templates
typedef double pkey_t;   // key type   [Same as muse::Time] - see KEY_MAX above
typedef void   *pval_t;  // value type [Generic pointer]
typedef int    pid_t;    // id type    [Same as muse::AgentID] - unique id

//Node type, the structure for a single node in the queue
struct Node_t {
    pkey_t  key;       // value for computing priority in queue
    pid_t   id;        // a unique id for entries to allow insert w/ same key
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
     * This operation is NOT thread safe, and should only be active after 
     * all operations are done with the queue.
     */
    ~LockFreePQ();
    
    /**
     * Inserts a new value into queue, with duplicate (key, id) entries skipped
     * 
     * If an entry in the queue exists with the same key+id, the value will
     * NOT be inserted and instead the conflicting value in the queue returned.
     * If an insert is not expected to have a duplicate already present, must
     * ensure the return value of this method is NULL to ensure it was inserted.
     * 
     * Note: Duplicate value that gets returned is not exclusive. That is to
     * say other threads may also access it concurrently, meaning there must
     * be additional logic higher up in the stack to handle the return value
     * in a thread safe way.
     * 
     * This method is thread safe and can be called concurrently with other
     * threads with no risk of being lost or being inserted out of order
     * 
     * @param key - pkey_t type, the value with which priority is calculated
     * @param value - the value of this entry in the queue
     * @param id - the id of the entry to prevent duplicates, default to -1
     * @return duplicate - value already present in the queue at key, else NULL
     */
    pval_t insert(pkey_t key, pval_t value, pid_t id = -1);
    
    
    /**
     * Removes and returns the value in the queue with the lowest key
     * 
     * If the queue is empty, this will return null
     * 
     * This method is thread safe and ensures the minimum value of the queue
     * at the moment the method is called is returned exclusively to the caller
     * 
     * @return value - the value of lowest priority node in the queue
     */
    pval_t deleteMin();
    
    /**
     * Returns the key of the node with the next lowest key (front) in the queue
     * 
     * Does not modify the queue in any way
     * 
     * Returns KEY_MAX-1 if queue is empty (such that returned key could still 
     * be used to insert into the back of the queue)
     * _______________________________________________
     * ========= ***** NOT THREAD SAFE ***** =========
     * 
     * Return value could be incorrect the moment it is returned if
     * another thread was modifying the queue concurrently when this was called.
     * This means this method should NOT be used unless other measures to ensure 
     * thread safety are met higher up in the stack.
     * 
     * @return value - the next key in the priority queue
     */
    pkey_t nextMin();
    
    /**
     * Searches the queue for a key+id and returns the value associated with it
     * 
     * If value not found, returns null
     * 
     * This method does ***NOT ENSURE EXCLUSIVE ACCESS*** to the element. Other 
     * threads could call this method and manipulate the value concurrently.
     * As a result thread safety must be handled higher up the stack when
     * this method is used.
     * 
     * Key must be exact match of key in the queue, this can be an issue with
     * floating point values such as doubles.
     * 
     * @param k - key to search for
     * @param id - id to search for
     * @return value at key+id, if not found null
     */
    pval_t getEntry(pkey_t k, pid_t id = -1);
    
    /**
     * Returns the first (least) Node with key+id >= given key+id for traversal
     * 
     * Returns tail if empty
     * _______________________________________________
     * ========= ***** NOT THREAD SAFE ***** =========
     * 
     * This method can only be used when it is guarenteed that nodes won't
     * be deleted, as if nodes get deleted by another thread this reference
     * becomes invalid to traverse
     * 
     * *** NOTE: the next[] reference may refer to a node that was deleted, 
     * if so the value of the node will be null. Since no thread is deleting
     * when using this reference, its safe to assume that if a node has a
     * value, that value will be valid until this thread is done with it
     * 
     * @return the first valid node in this queue, null if empty
     */
    Node_t* getNode(pkey_t k, pid_t id = -1);
    
    /**
     * For traversal, pointer to tail of queue
     * 
     * @return pointer to tail of queue
     */
    Node_t* tail() {
        return tail;
    }
    
    /**
     * Searches the queue for a key+id, deletes it, and returns associated value
     * 
     * If value not found, returns null
     * 
     * This method ensures exclusive access to the returned value
     * 
     * Key must be exact match of key in the queue, this can be an issue with
     * floating point values such as doubles.
     * 
     * @param k - key to search for
     * @param id - id to search for
     * @return value at key+id, if not found null
     */
    pval_t deleteEntry(pkey_t k, pid_t id = -1);
    
//    /**
//     * Removes all values greater than or equal to a given key value
//     * 
//     * @param k - The key after which all entries should be deleted (inclusive)
//     */
//    void deleteAfter(pkey_t k, pid_t id = -1);
    
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
     * Gets the predecessors and successors for key k at all skip list levels
     * 
     * This information is used by the insert method to attempt to quickly
     * insert the node in the queue. It's possible these preds and succs have
     * changed after this is called, so must use compare and swap to actually
     * do the insert.
     * 
     * For example, preds[4] is a ptr to the node that would preceed a node of 
     * key value k at skip-list level 4 (where first level is 0)
     * 
     * @param k     - the key that we want predecessors and successors for
     * @param id    - the id we want preds/succs for, secondary sort parameter
     * @param preds - list of predecessors at each level for key value k
     * @param succs - list of successors at each level for key value k
     * 
     * @return del - the last deleted node on the bottom level, could be null
     */
    Node_t* locatePreds(pkey_t k, pid_t id, Node_t ** preds, Node_t ** succs);
    
    /**
     * Restructures higher level pointers following a deleteMin() operation
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
     * Node starts with inserting flag set to true, must be set to false
     * when done inserting the returned node.
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
     * For example, the 3rd thread in the system gets thread_id = 0b100 = 4
     */
    thread_local static size_t thread_id;
    static size_t *lastId;  // used to distribute IDs to threads
    
    /**
     * bitmap which represents which threads are acitvely in critical sections
     * 
     * Each bit represents a thread actively in a critical section, when a 
     * thread leaves its critical section it flips its bit back to 0
     * 
     * For example, if state = 0b0110 then two threads are actively in crit secs
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
    
};


#endif /* LOCK_FREE_PQ_H */

