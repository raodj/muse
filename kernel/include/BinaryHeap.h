#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

//---------------------------------------------------------------------------
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
// Authors: Meseret Gebre          meseret.gebre@gmail.com
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <vector>
#include <algorithm>
#include "DataTypes.h"
#include "EventQueue.h"

BEGIN_NAMESPACE(muse);

/** \brief Binary (min) Heap based on std::vector

    This class provides a convenient interface to a vector-backed
    heap.  The heap is created and managed using standard methods.
    This class provides a generic heap of arbitrary object T.  The
    heap also accepts an custom comparator to be used to order the
    items in the heap.

    \note Note that if the heap is used to contains pointers to
    objects, then the pointers are not deleted/free'd by heap.  If
    using pointers, it is best to use a std::shared_ptr with this
    heap.
*/
template <typename T, typename Compare = std::less<T>>
class BinaryHeap {
public:
    /** \brief Default constructor.

        The constructor initializes the vector and creates the
        initial, empty heap.
     */
    BinaryHeap(){ 
        heapContainer = new std::vector<T>;
         std::make_heap(heapContainer->begin(), heapContainer->end(), comp);
    }
    /** \brief Destructor.

        The destructor deletes all allocated memory, in this case, the
        vector.  Note that if the heap contains pointers, then the
        pointers are not deleted by heap.  If using pointers, it is
        best to use a std::shared_ptr with this heap.
     */
    ~BinaryHeap() { delete heapContainer; }

    /** \brief Get the top element of the heap

        This method returns a reference to the current top element of
        the heap.

        \return A reference to the top value.
    */
    inline T& top() const { return heapContainer->front(); }
    
    /** Remove all objects in this heap.

        This is a convenience method that is used to remove all
        pending objects in this heap.  This method is handy when an
        agent is logically removed from a scheduler queue.
    */
    void clear() {
         ASSERT( heapContainer != NULL );
         heapContainer->clear();
    }

    /** \brief Remove the top element from the heap

        This method will remove the current top element from the heap,
        and then fixup the heap.
    */
    void pop() {
        if (heapContainer->empty()) {
            return;
        }
        std::pop_heap(heapContainer->begin(), heapContainer->end(), comp);
        heapContainer->back();
        heapContainer->pop_back();
    }

    /** \brief Push an element onto the heap

        This method will add the specified element to the heap.

        \note This class will not call increaseReference() etc. on the
        object being added.  Consequently, it is the responsibility of
        the caller to perform such reference-counting operations.
        
        \param[in] value The value to be push into the heap
    */
    void push(T& value) {
        // Note that any increase in reference counts needs to be done
        // prior to calling this method.
        heapContainer->push_back(value);
        std::push_heap(heapContainer->begin(), heapContainer->end(), comp);
    }

    /** \brief Push an vector of values onto the heap.

        This method will add all the values in the given vector onto
        to the heap.

        \note Using this method to add a whole bunch of values is more
        efficient that adding one value at a time.

        \note It is the callers responsibility to call
        increaseReference() as needed, on each value before invoking
        this method (preventing it from being deleted/garbage
        collected).

        \param[in] values The set of values to be added to the heap
    */
    void push(std::vector<T>& values) {
        // Due to bulk adding, ensure that the heap container has enough
        // capacity.
        heapContainer->reserve(heapContainer->size() + values.size() + 1);
        // Add each value item to the container.
        for(auto curr = values.begin(); (curr != values.end()); curr++) {
            T val = *curr;
            heapContainer->push_back(val);
        }
        std::make_heap(heapContainer->begin(), heapContainer->end(), comp);
        // Clear out values in the container as per API expectations
        values.clear();
    }
    
    /** \brief Remove all objects that match a given predicate from
        the heap.  

        This method will compare the timestamps of all events in the
        heap with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.

        \param pred The predicate to be used to determine if a value
        should be removed from the heap.

        \return This method returns the number of events actually
        removed.        
    */
    template<typename UnaryPredicate>
    int remove(UnaryPredicate pred) {
        int  numRemoved = 0;
        long currIdx    = heapContainer->size() - 1; 
        
        // NOTE: Here the heap is sorted based on different comparator
        // (for example: receive time for scheduling).  However, we
        // are canceling based on a different predicate (for example,
        // one that compares agent ID and sentTime).  Consequently,
        // doing any clever optimizations to minimize iterations will
        // backfire!
        while (!heapContainer->empty() && (currIdx >= 0)) {
            ASSERT(currIdx < (long) heapContainer->size());
            // A value is deleted only if it matches a given predicate
            // -- for example: An event is removed only if the *sent*
            // time is greater than the antiMessage's and if the event
            // is from same sender
            if (pred((*heapContainer)[currIdx])) {
                numRemoved++;
                // Now it is time to patchup the hole and fix up the heap.
                // To patch-up we move event from the bottom up to this
                // slot and then fix-up the heap.
                (*heapContainer)[currIdx] = heapContainer->back();
                heapContainer->pop_back();
                EventQueue::fixHeap(*heapContainer, currIdx, comp);
                // Update the current index so that it is within bounds.
                currIdx = std::min<long>(currIdx, heapContainer->size() - 1);
            } else {
                // Check the previous element in the vector to see if that
                // is a candidate for removal.
                currIdx--;
            }
        }
        // Return number of values removed to track statistics. 
        return numRemoved;
    }
    
    void remove(long index) {
        // Delete object and fix up the heap.
        // To patch-up we move object from the bottom up this 
        // slot and then fix-up the heap.
        (*heapContainer)[index] = heapContainer->back();
        heapContainer->pop_back();
        EventQueue::fixHeap(*heapContainer, index, comp);
    }
    
    /** \brief Get the current size of the heap

        \return The current size of the heap        
     */
    inline typename std::vector<T>::size_type size() const {
        return heapContainer->size();
    }

    /** \brief Check if the heap is empty

        \return True if the heap is empty
     */
    inline bool empty() const { return heapContainer->empty(); }
    
    /** \brief Get the iterator referring to the beginning of the element in the
         vector container.
      
       \return An iterator to the beginning element of the sequence. 
     */
    inline typename std::vector<T>::iterator begin() {
        return heapContainer->begin();
    }
    
    /** \brief Get the iterator referring to past the end element in the
         vector container.
      
       \return An iterator to the element past the end of the sequence. 
     */
    inline typename std::vector<T>::iterator end() {
        return heapContainer->end();
    }
    
    /** \brief Get the reverse iterator referring to past the end element in the
         vector container.
      
       \return A reverse iterator to the element past the end of the sequence. 
     */
    inline typename std::vector<T>::reverse_iterator rbegin() {
        return heapContainer->rbegin();
    }
    
    /** \brief Get the reverse iterator referring to the beginning element in the
         vector container.
      
       \return A reverse iterator to the beginning element of the sequence. 
     */    
    inline typename std::vector<T>::reverse_iterator rend() {
        return heapContainer->rend();
    }
    
    /** Convenience method to return receive time of the lowest
        timestamp event in this heap (for a given agent).
        
        Simply returns the receive Time of the top event in this
        heap. If heap is empty then TIME_INIFINITY is returned.
     
        \return The receive time of top event recv time or
        TIME_INFINITY if heap is empty.
    */
    inline muse::Time getTopTime() const {
        return empty() ? TIME_INFINITY : top().getReceiveTime();
    }
\
    /** \brief This method is used to search for a specific element
         stored on the heap.
      
        \return iterator to the element, if the specified element is found or 
         heapContainer::end(), if the searched element is not found
     */
    typename std::vector<T>::iterator find(const T& value) {
        typename std::vector<T>::iterator curr = heapContainer->begin();
        while(curr!= heapContainer->end()) {
            if(*curr == value) {
                return curr;
            }
            curr++;
        }
        return curr;
    }
    
    /** Print values in this heap to a given output stream.

        This method prints all the values currently in the heap to a
        given output stream.  This is primarily used for debugging
        purposes.

        \param[out] os The output stream to which the values are to be
        written.
    */
    void print(std::ostream& os) const {
        for(auto curr = heapContainer->begin(); curr!= heapContainer->end();
                curr++) {
            T val = *curr;
            os << val << std::endl;
        }
    }

protected:
    /** The default comparator object to be used to order values in
        this heap. */
    Compare comp;

private:
    /// The vector backing the heap
    std::vector<T>* heapContainer;
};

END_NAMESPACE(muse);

#endif
