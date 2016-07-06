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
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include <vector>
#include <algorithm>
#include "Event.h"
#include "EventQueue.h"
#include "ThreeTierHeapEventQueue.h"

BEGIN_NAMESPACE(muse);

using namespace muse;

/** \brief Binary (min) Heap based on std::vector

    This generic class provides a convenient interface to a vector-backed
    heap. The heap is created and managed using standard methods.

    
*/
template <typename T>
class BinaryHeap {
public:
    /** \brief Default constructor.

        The constructor initializes the vector and creates the
        initial, empty heap.
     */
    BinaryHeap();

    /** \brief Destructor.

        The destructor deletes all allocated memory, in this case, the
        vector.
     */
    ~BinaryHeap();

    /** \brief Get the top element of the heap

        This method returns a pointer to the current top element of
        the heap.

        \return A pointer to the top Event
    */
    inline T* top() const { return heapContainer->front(); }

    /** \brief Remove the top element from the heap

        This method will remove the current top element from the heap,
        and then fix-up the heap.

        \note This method does not return the top element from the
        heap -- however, calling this method may cause the Event to be
        deleted as it will decreaseReference().
    */
    void pop();

    /** \brief Push an element onto the heap

        This method will add the specified element to the heap.

        \note This class will call increaseReference() on the event,
        preventing it from being automatically deleted.

        \param[in] event The event to be push into the heap
    */
    template <typename T2>
    void push(T2* event);
    
    /** \brief Push an set of events onto the heap.

        This method will add all events in the given event container
        onto to the heap.

        \note Using this method to add a whole bunch of events is more
        efficient that adding one event at a time.

        \note This class will call increaseReference() on each event
        in the given event container (preventing it from being
        deleted/garbage collected).

        \param[in] events The set of events to be added to the heap
    */
    template <typename T2>
    void push(T& events);
    
    /** \brief Push a TierThree object onto the heap.

        This method will add a TierThree object in the given Tier2 container
        onto to the heap.

        \param[in] Tier2 The vector of TierThree objects to be added to the heap
    */
    template <typename T2>
    void pushObj(T2& tierThree);
    
        /** \brief Push a set of TierThree objects onto the heap.

        This method will add all the TierThree objects in the given Tier2
        container onto to the heap.

        \param[in] Tier2 The vector of TierThree objects to be added to the heap
    */
    void pushObjs(T& tierTwo);
    
    
    /** \brief Remove all events that occur after the specified Event

        This method will compare the timestamps of all events in the
        heap with that of the specified event. Any Event (from the
        same sender) that occurs at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] antiMsg The Anti-Message indicating what should be
        removed
        
        \return True if any cancellation was done
    */
    template <typename T2>
    bool removeFutureEvents(const T2* antiMsg);

    /** \brief Remove all events from a given sender that have been
        sent after a specified time.

        This method will compare the timestamps of all events in the
        heap with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.

        \param[in] sender The ID of the agent whose events have to be
        removed.  This agent must be a valid agent that has been
        registered/added to this event queue.  The pointer cannot be
        NULL.

        \param[in] sentTime The time from which the events are to be
        removed.  All events (including those sent at this time) sent
        by the sender agent are removed from this event queue.

        \return This method returns the number of events actually
        removed.        
    */
    template <typename T2>
    int removeFutureEvents(const muse::AgentID sender,
                           const muse::Time sentTime);
    
    /** \brief Get the current size of the heap

        \return The current size of the heap        
     */
    inline typename T::size_type size() const {
        return heapContainer->size();
    }

    /** \brief Check if the heap is empty

        \return True if the heap is empty
     */
    inline bool empty() const { return heapContainer->empty(); }
    
    /** Print events in this heap to a given output stream.

        This method prints all the events currently in the heap to a
        given output stream.  This is primarily used for debugging
        purposes.

        \param[out] os The output stream to which the events are to be
        written.
    */
    void print(std::ostream& os) const;
    
     /** Print tier2 contents stored in the heap to a given output stream.

        This method prints TierThree objects currently in the heap to a
        given output stream. This is primarily used for debugging purposes.

        \param[out] os The output stream to which the events are to be
        written.
    */
    void printObjs(std::ostream& os) const;
    
protected:
    // Currently, this class does not have any protected members

private:
    /// The vector backing the heap
    T* heapContainer;
    
    /** \brief Event Comparator for min-heap creation

        This class provides a function for comparing two Event
        instances. It is used internally for heap operations.

        Events are ordered by their receive times.
        
    */
    class EventComp {
    public:
        inline EventComp() {}
        template <typename T2>
        inline bool operator()(const T2* const lhs,
                               const T2* const rhs) const {
            const Time lhs_time = lhs->getReceiveTime(); 
            return (lhs_time > rhs->getReceiveTime()); 
        }
        inline bool operator()(TierThree lhs,  TierThree rhs) const {
            Time lhs_time = lhs.getRecvTime();
            return (lhs_time > rhs.getRecvTime()); 
        }
    };
};

template <typename T>
BinaryHeap<T>::BinaryHeap() {
    heapContainer = new T;
    std::make_heap(heapContainer->begin(), heapContainer->end(), EventComp());
}

template <typename T>
BinaryHeap<T>::~BinaryHeap() {
    delete heapContainer;
}

template <typename T>
void BinaryHeap<T>::pop() {
    if (heapContainer->empty()) {
	return;
    }
    std::pop_heap(heapContainer->begin(), heapContainer->end(), EventComp());
    heapContainer->back()->decreaseReference();
    heapContainer->pop_back();
}

template <typename T>
template <typename T2>
void BinaryHeap<T>::push(T2 *event) {
    event->increaseReference();
    heapContainer->push_back(event);
    std::push_heap(heapContainer->begin(), heapContainer->end(), EventComp());
}

template <typename T>
template <typename T2>
void BinaryHeap<T>::push(T& events) {
    // Due to bulk adding, ensure that the heap container has enough
    // capacity.
    heapContainer->reserve(heapContainer->size() + events.size() + 1);
    // Add all the events to the container.
    for(typename T::iterator curr = events.begin(); (curr != events.end());
        curr++) {
        T2* event = *curr;
        heapContainer->push_back(event);
    }
    std::make_heap(heapContainer->begin(), heapContainer->end(), EventComp());
    // Clear out events in the container as per API expectations
    events.clear();
}

template <typename T>
template <typename T2>
void BinaryHeap<T>::pushObj(T2& tierThree) {
    heapContainer->push_back(tierThree);
    std::push_heap(heapContainer->begin(), heapContainer->end(), EventComp());
}

template <typename T>
void BinaryHeap<T>::pushObjs(T& tierTwo) {
    heapContainer->reserve(heapContainer->size() + tierTwo.size() + 1);
    for(typename T::iterator it = tierTwo.begin(); it != tierTwo.end(); it++) {
        TierThree& current = *it;  
        heapContainer->push_back(current);
    }
    std::make_heap(heapContainer->begin(), heapContainer->end(), EventComp());
}

template <typename T>
void BinaryHeap<T>::print(std::ostream& os) const {
    for(typename T::const_iterator curr = heapContainer->cbegin();
        (curr != heapContainer->end()); curr++) {
        os << **curr << std::endl;
    }
}

template <typename T>
void BinaryHeap<T>::printObjs(std::ostream& os) const {
    for(typename T::iterator it = heapContainer->begin();
        (it != heapContainer->end()); it++) {
         muse::TierThree& current = *it;  
         os <<"[Agent ID: " << current.getAgentID() << ", "
            <<"Receive Time: " << current.getRecvTime()
            << "]" << std::endl;
    }
}

template <typename T>
template <typename T2>
bool BinaryHeap<T>::removeFutureEvents(const T2* antiMsg) {
    const int numRemoved = removeFutureEvents(antiMsg->getSenderAgentID(),
                                              antiMsg->getSentTime());
    return (numRemoved > 0);
}

template <typename T>
template <typename T2>
int BinaryHeap<T>::removeFutureEvents(const muse::AgentID sender,
                                      const muse::Time sentTime) {
    EventComp comparator;   // Event comparator used further below.
    int  numRemoved = 0;
    long currIdx    = heapContainer->size() - 1;

    // NOTE: Here the heap is sorted based on receive time for
    // scheduling.  However, we are canceling based on sentTime.
    // Consequently, doing any clever optimizations to minimize
    // iterations will backfire!
    while (!heapContainer->empty() && (currIdx >= 0)) {
        ASSERT(currIdx < (long) heapContainer->size());
        T2* const evt = (*heapContainer)[currIdx];
        ASSERT(evt != NULL);
        // An event is deleted only if the *sent* time is greater than
        // the antiMessage's and if the event is from same sender
        if ((evt->getSenderAgentID() == sender) &&
            (evt->getSentTime() >= sentTime)) {
            // This event needs to be cancelled.
            evt->decreaseReference();
            numRemoved++;
            // Now it is time to patchup the hole and fix up the heap.
            // To patch-up we move event from the bottom up to this
            // slot and then fix-up the heap.
            (*heapContainer)[currIdx] = heapContainer->back();
            heapContainer->pop_back();
            EventQueue::fixHeap(*heapContainer, currIdx, comparator);
            // Update the current index so that it is within bounds.
            currIdx = std::min<long>(currIdx, heapContainer->size() - 1);
        } else {
            // Check the previous element in the vector to see if that
            // is a candidate for cancellation.
            currIdx--;
        }
    }
    // Return number of events canceled to track statistics. 
    return numRemoved;
}

END_NAMESPACE(muse);

#endif
