#ifndef BINARY_HEAP_WRAPPER_CPP
#define BINARY_HEAP_WRAPPER_CPP

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

#include <algorithm>
#include "EventQueue.h"
#include "BinaryHeapWrapper.h"


using namespace muse;
using std::make_heap;
using std::pop_heap;
using std::push_heap;

BinaryHeapWrapper::BinaryHeapWrapper() {
    heapContainer = new EventContainer;
    make_heap(heapContainer->begin(), heapContainer->end(), EventComp());
}

BinaryHeapWrapper::~BinaryHeapWrapper() {
    delete heapContainer;
}

void
BinaryHeapWrapper::pop() {
    if (heapContainer->empty()) {
	return;
    }
    pop_heap(heapContainer->begin(), heapContainer->end(), EventComp());
    EventQueue::decreaseReference(heapContainer->back());
    heapContainer->pop_back();
}

void
BinaryHeapWrapper::push(Event * event) {
    EventQueue::increaseReference(event);
    heapContainer->push_back(event);
    push_heap(heapContainer->begin(), heapContainer->end(), EventComp());
}

void
BinaryHeapWrapper::push(EventContainer& events) {
    // Due to bulk adding, ensure that the heap container has enough
    // capacity.
    heapContainer->reserve(heapContainer->size() + events.size() + 1);
    // Add all the events to the conainer.
    for(EventContainer::iterator curr = events.begin(); (curr != events.end());
        curr++) {
        muse::Event* event = *curr;
        heapContainer->push_back(event);
    }
    make_heap(heapContainer->begin(), heapContainer->end(), EventComp());
    // Clear out events in the container as per API expectations
    events.clear();
}

bool
BinaryHeapWrapper::removeFutureEvents(const Event* antiMsg) {
    const int numRemoved = removeFutureEvents(antiMsg->getSenderAgentID(),
                                              antiMsg->getSentTime());
    return (numRemoved > 0);
}

#ifdef REBUILD_HEAP
int
BinaryHeapWrapper::removeFutureEvents(const muse::AgentID sender,
                                      const muse::Time sentTime) {
    int numRemoved = 0;
    EventContainer* temp = new EventContainer;
    
    while (!heapContainer->empty()) {
        // First, get a pointer to the last event, and remove it from
        // the container (it will be either deleted or requeued as
        // necessary)
        Event* lastEvent = heapContainer->back();
        heapContainer->pop_back();
        // An event is deleted only if the *sent* time is greater than
        // the antiMessage's and if the event is from same sender
        if ((lastEvent->getSenderAgentID() == sender) &&
            (lastEvent->getSentTime() >= sentTime)) {
            // This event needs to be cancelled
            EventQueue::decreaseReference(lastEvent);
            numRemoved++;
        } else {
            // This event is still valid, and needs to be processed
            temp->push_back(lastEvent);
        }
    }

    ASSERT (heapContainer->empty());
    // Replace the container with the new one we just created
    delete heapContainer;
    heapContainer = temp;
    
    // Fix up the heap (if it is not empty)
    if (!heapContainer->empty()) {
        make_heap(heapContainer->begin(), heapContainer->end(), EventComp());
    }
    return numRemoved;
}
#else
int
BinaryHeapWrapper::removeFutureEvents(const muse::AgentID sender,
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
        Event* const evt = (*heapContainer)[currIdx];
        ASSERT(evt != NULL);
        // An event is deleted only if the *sent* time is greater than
        // the antiMessage's and if the event is from same sender
        if ((evt->getSenderAgentID() == sender) &&
            (evt->getSentTime() >= sentTime)) {
            // This event needs to be cancelled.
            EventQueue::decreaseReference(evt);
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
#endif  // REBUILD_HEAP

void
BinaryHeapWrapper::print(std::ostream& os) const {
    for(EventContainer::const_iterator curr = heapContainer->cbegin();
        (curr != heapContainer->end()); curr++) {
        os << **curr << std::endl;
    }
}

void
BinaryHeapWrapper::clear() {
    ASSERT( heapContainer != NULL );
    // Decrease reference for all events in our event container
    for (Event* evt : *heapContainer) {
        EventQueue::decreaseReference(evt);
    }
    // Clear out our event container
    heapContainer->clear();
}

#endif
