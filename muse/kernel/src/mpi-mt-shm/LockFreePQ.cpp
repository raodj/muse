
#include "kernel/include/mpi-mt-shm/LockFreePQ.h"



// The ID associated with any thread that accesses this queue
template <class K, class V, class Compare>
thread_local size_t LockFreePQ<K, V, Compare>::thread_id = THREAD_ID_NULL;

// A static value used to distribute unique IDs across multiple threads
template <class K, class V, class Compare>
size_t* LockFreePQ<K, V, Compare>::lastId = new size_t(0);

template <class K, class V, class Compare>
LockFreePQ<K, V, Compare>::LockFreePQ() {
    
    comp = Compare();
    
    head = new Node_t;
    tail = new Node_t;
    
    // (deperomm): keyMin/keyMax must be set as static members during some init
    //             better way?
//    assert(keyMin != NULL);
//    assert(keyMax != NULL);
    
    // Set initial values
    head->inserting = 0;
    tail->inserting = 0;
    head->key       = keyMin;
    tail->key       = keyMax;
    head->level     = NUM_LEVELS - 1;
    tail->level     = NUM_LEVELS - 1;
    head->value     = NULL;
    tail->value     = NULL;
    
    // At all levels of the skip list, connect head and tail
    for (size_t i = 0; i < NUM_LEVELS; i++) {
        head->next[i] = tail;
    }
    
    maxOffset = MAX_OFFSET; // todo(deperomm): make this cmd line arg?
}

template <class K, class V, class Compare>
LockFreePQ<K, V, Compare>::~LockFreePQ() {
    
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

template <class K, class V, class Compare>
V
LockFreePQ<K, V, Compare>::insert(K key, V val) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // new key cannot be equal to head or tail node key values
    assert(comp(keyMin, key));
    assert(comp(key,    keyMax));
    
    // value cannot start null as we set it to null if deleted mid queue
    assert(val != NULL);
    
    Node_t *newNode = allocNode();
    Node_t *del     = NULL;
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    
    newNode->key   = key;
    newNode->value = val;
    
    // Insert into bottom level
    bool inserted;
    do {
        del = locatePreds(key, preds, succs);
        
        // if key already exists
        if (!comp(succs[0]->key, key) && !comp(key, succs[0]->key)
                && !is_marked_ref(preds[0]->next[0]) 
                && preds[0]->next[0] == succs[0]) {
            
            V ret = succs[0]->value;
            
            if (ret == NULL) {
                // this entry was popped off previously, re-use the entry
                if(__sync_bool_compare_and_swap(&succs[0]->value, 
                        NULL, val)) {
                    // successfully inserted back into this old node
                    exitCritical();
                    return NULL;
                } else {
                    ret = succs[0]->value;
                    assert(ret != NULL); // this is possible?...
                }
            }
            freeNode(newNode); // we didn't insert anything
            
            exitCritical();
            return ret; // return the entry that matches the key
                                    // this could be null if it was deleted
        }
        
        // begin inserting
        newNode->next[0] = succs[0];
        
        // We use compare and swap to ensure thread safety, if fails restart
        inserted = 
            __sync_bool_compare_and_swap(&preds[0]->next[0], succs[0], newNode);
        
    } while (!inserted);
    
    // Insert at all higher levels
    int i = 1;
    while (i <= newNode->level) {
        // If at this stage we find a successor marked for deletion, we can
        // stop inserting at higher levels and just continue
        if (is_marked_ref(newNode->next[0]) ||
            is_marked_ref(succs[i]->next[0]) ||
            del == succs[i])
            break;

        // begin inserting
        newNode->next[i] = succs[i];
        if (!__sync_bool_compare_and_swap(&preds[i]->next[i], succs[i], newNode))
        {
            // failed b/c competing insert or restructure, try this level again
            del = locatePreds(key, preds, succs);

            // if new node has been deleted, we're done
            if (succs[0] != newNode) break;
	    
        } else {
            // Succeeded at this level, move on to next
            i++;
        }
    }
    
    if (newNode) {
        newNode->inserting = 0;
    }
    
    exitCritical();
    return NULL;
}

template <class K, class V, class Compare>
V
LockFreePQ<K, V, Compare>::deleteEntry(K key) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // new key cannot be equal to head or tail node key values
    assert(comp(keyMin, key));
    assert(comp(key,    keyMax));
    
    V ret = NULL;
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    
    locatePreds(key, preds, succs);

    // attempt to delete/return if we found the key+id pair
    if (!comp(succs[0]->key, key) && !comp(key, succs[0]->key)
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

template <class K, class V, class Compare>
V
LockFreePQ<K, V, Compare>::deleteMin() {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // initial variables
    V ret = NULL;
    Node_t *pred, *succ, *cur, *obsHead = NULL, *newHead = NULL;
    
    int offset = 0;
    
    pred = head;
    // first physical item in the queue. could be deleted, could be tail
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
        // even if it was already deleted during that process
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
    assert(pred->value == NULL); // should have been set to null above
    
                               //(unless another thread already re-inserted it?)
    
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

template <class K, class V, class Compare>
K
LockFreePQ<K, V, Compare>::nextMin() {
    // This method must call exitCritical() before returning
    enterCritical();
    
    // initial variables
    K ret = keyMaxMinusOne;
    Node_t *pred, *succ;
    
    pred = head;
    
    do { 
        // likely expensive - high probability of a cache miss here, as with all
        // linked list traversals
        succ = pred->next[0];
        
        // if list is empty, we return KEY_MAX-1
        if (get_unmarked_ref(succ) == tail) {
            exitCritical();
            return ret; // KEY_MAX
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

template <class K, class V, class Compare>
V
LockFreePQ<K, V, Compare>::getEntry(K key) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    assert(comp(keyMin, key));
    assert(comp(key,    keyMax));
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    

    locatePreds(key, preds, succs);

    // return if key-id pair was NOT found (since it must exist in the
    // queue in order for us to delete it)
    if ( ( !comp(succs[0]->key, key) && !comp(key, succs[0]->key) )
            || is_marked_ref(preds[0]->next[0]) 
            || preds[0]->next[0] != succs[0]) {

        exitCritical();
        return NULL; 
    }
    
    // this could be null if deleted, that's okay, we return null if not found
    // this will be tail if the queue is empty, which has value of NULL
    V ret = succs[0]->value;
    
    exitCritical();
    return ret;
}

template <class K, class V, class Compare>
typename LockFreePQ<K, V, Compare>::Node_t*
LockFreePQ<K, V, Compare>::getNode(K key) {
    // This method must call exitCritical() before returning
    enterCritical();
    
    assert(comp(keyMin, key));
    assert(comp(key,    keyMax));
    
    Node_t *preds[NUM_LEVELS], *succs[NUM_LEVELS];
    

    locatePreds(key, preds, succs);
    
    assert(succs[0] != NULL);

    exitCritical();
    return succs[0];
}

//void
//LockFreePQ<K, V, Compare>::deleteAfter(pkey_t k, pid_t id) {
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

template <class K, class V, class Compare>
typename LockFreePQ<K, V, Compare>::Node_t*
LockFreePQ<K, V, Compare>::locatePreds(K key, 
        Node_t ** preds, Node_t ** succs) {
    
    Node_t *pred, *succ, *del = NULL;
    int  i;
    bool wasDeleted;
    
    assert(comp(keyMin, key));
    assert(comp(key,    keyMax));
    
    pred = head;
    
    // traverse the skip list top down, getting pred and succ at each level
    i = NUM_LEVELS - 1;
    while (i >= 0) {
        // traverse the first node at this level
        succ = pred->next[i];
        
        assert(succ != NULL);
        
        // because delete flag is on the pointer, need to store if this
        // node is deleted before we get the unmarked pointer to actually use
        wasDeleted = is_marked_ref(succ);
        succ = get_unmarked_ref(succ);
        
        // traverse all nodes at this level until we find where we belong
        // note that we continue if
        //     1. the current successor's key+id pair is less than the given one
        //     2. the current successor has nodes after it that are deleted
        //     3. on bottom level: the current successor was marked for delete
        // succ is what we insert behind and must not be deleted
        while (comp(succ->key, key) || is_marked_ref(succ->next[0]) 
                || ((i == 0) && wasDeleted)) {
            
            // on the bottom level, record the last seen deleted node
            if (i == 0 && wasDeleted)
                del = succ;
            pred = succ;
            succ = pred->next[i];
            wasDeleted = is_marked_ref(succ);       
            succ = get_unmarked_ref(succ);
            assert(succ != NULL);
            if (succ == NULL) {
                // This is a serious issue. It appears to be related to
                // garbage collection, where elements of the list are freed
                // before all threads were done operating on them.
                // This assert got triggered very rarely (1 in ~20 runs with
                // millions of elements at very high contention), so if we 
                // reach this point we simply try again to alleviate the issue 
                // for now.
                // todo(deperomm): review garbage collection
                return locatePreds(key, preds, succs);
            }
        }
        preds[i] = pred;
        succs[i] = succ;
        i--;
    }
    return del;
}

template <class K, class V, class Compare>
void
LockFreePQ<K, V, Compare>::restructure() {
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

template <class K, class V, class Compare>
typename LockFreePQ<K, V, Compare>::Node_t*
LockFreePQ<K, V, Compare>::allocNode() {
    Node_t *newNode;
    
    // In skip lists, the level of a node is randomly decided when created
    // We want each level up to be half as likely as the level below it, so
    // a geometric distribution
    // todo(deperomm): Validate this random generator is working sufficently
    static thread_local std::mt19937 generator;
    static thread_local std::geometric_distribution<int> distribution;
    
    int level = distribution(generator);
    if (level > NUM_LEVELS) {
        // wow, this should only happen 1 in 2^32 (4 trillion) times...
        level = NUM_LEVELS - 1;
    }
    
    assert(0 <= level && level < NUM_LEVELS);
    
    // Allocate the new node w/ enough space to hold pointers to (level) ptrs
    // sizeof *new_node has enough space for bot level, then + space for others
    // note bottom level is 0, so any higher level adds space to the node
    // todo(deperomm): Make this come from a garbage collector/recycler
    newNode = new Node_t;
    
    newNode->level     = level;
    newNode->inserting = 1; // nodes always start off as being inserted

    return newNode;
}

template <class K, class V, class Compare>
void 
LockFreePQ<K, V, Compare>::freeNode(Node_t *n) {
    std::lock_guard<std::mutex> guard(freeMutex);
    pendingDeallocs->push_back(n);
}

template <class K, class V, class Compare>
void
LockFreePQ<K, V, Compare>::enterCritical() {
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

template <class K, class V, class Compare>
void
LockFreePQ<K, V, Compare>::exitCritical() {
    
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

template <class K, class V, class Compare>
void
LockFreePQ<K, V, Compare>::garbageCollectAndChangeEpoch() {    
    
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
    pendingDeallocs->swap(*waitingDeallocs);
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

template <class K, class V, class Compare>
void
LockFreePQ<K, V, Compare>::finalizeDeallocs() {    
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

template <class K, class V, class Compare>
void
LockFreePQ<K, V, Compare>::print() {
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
//void test(LockFreePQ<int, int*> *pq, std::vector<int> vals) {
//    long out = 0;
//    long in = 0;
//    int x;
//    for (size_t i = 0; (i+1) < vals.size(); i+=2) {
//        x = vals[i];
//        in += x;
//        int *val = new int(x);
//        pq->insert(x, val);
//        val = pq->deleteEntry(x);
//        if (val != NULL) {
//            out += *val;
//            delete  val;
////            std::cout << "O "; // debug print if value was found
//        } else {
//            val = pq->deleteMin();
//            out += *val;
//            delete  val;
////            std::cout << "XXXX "; // debug print if value was not found
//        }
//        x = vals[i+1];
//        in += x;
//        val = new int(x);
//        pq->insert(x, val);
//        val = pq->deleteMin();
//        out += *val;
//        delete  val;
//    }
//    std::cout << "Inserted: " << (long)in << " Got: " << (long)out << " Difference: " << ((long)in - (long)out) << std::endl;
//}
//
//int main(int argc, char** argv) {
//    
//    LockFreePQ<int, int*> *pq = new LockFreePQ<int, int*>;
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
//        k = 200000;
//    }
//    
//    std::vector<std::vector<int>> vals(n);
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