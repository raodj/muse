#ifndef HEAP_EVENT_QUEUE_CPP
#define HEAP_EVENT_QUEUE_CPP

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <algorithm>
#include "HeapEventQueue.h"

BEGIN_NAMESPACE(muse)

HeapEventQueue::HeapEventQueue() : EventQueue("HeapEventQueue"), maxQsize(0) {
    // Nothing else to be done.
}

muse::HeapEventQueue::~HeapEventQueue() {
    // Nothing else to be done.
}

void*
HeapEventQueue::addAgent(muse::Agent* agent) {
    UNUSED_PARAM(agent);
    return NULL;
}

muse::Event*
HeapEventQueue::front() {
    return !empty() ? eventList.front() : NULL;
}

void
HeapEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (eventList.empty()) {
        return;  // Nothing to be removed
    }
    // Initialize iterators
    const muse::Event*  nextEvt    = front();
    const muse::AgentID receiver   = nextEvt->getReceiverAgentID();
    const muse::Time    currTime   = nextEvt->getReceiveTime();
    // Remove all concurrent events for the next agent from the queue
    do {
        muse::Event* event = pop_front();
        events.push_back(event);
        nextEvt = (!empty() ? front() : NULL);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() && (nextEvt->getReceiverAgentID() == receiver) &&
             TIME_EQUALS(nextEvt->getReceiveTime(), currTime));
}

void
HeapEventQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    UNUSED_PARAM(agent);
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    event->increaseReference();
    eventList.push_back(event);
    maxQsize = std::max(maxQsize, eventList.size());
    std::push_heap(eventList.begin(), eventList.end(), compare);
}

void
HeapEventQueue::enqueue(muse::Agent* agent, muse::EventContainer& events) {
    UNUSED_PARAM(agent);
    ASSERT(agent != NULL);
    // Due to bulk adding, ensure that the heap container has enough
    // capacity.
    eventList.reserve(eventList.size() + events.size() + 1);
    // Add all the events to the conainer.
    for(EventContainer::iterator curr = events.begin(); (curr != events.end());
        curr++) {
        muse::Event* event = *curr;
        eventList.push_back(event);
        std::push_heap(eventList.begin(), eventList.end(), compare);
    }
    maxQsize = std::max(maxQsize, eventList.size());    
    // Clear out events in the container as per API expectations
    events.clear();
}

// #define REBUILD_HEAP 1
#ifdef REBUILD_HEAP
int
HeapEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                           const muse::Time sentTime) {
    UNUSED_PARAM(dest);
    ASSERT(dest != NULL);
    // Copy events to be retained into a temporary vector and finally
    // swap it with sel.
    size_t removedCount = 0;       // Count of events removed.
    std::vector<Event*> retained;  // Temporary vector with non-canceled events
    retained.reserve(eventList.size());
    for (auto curr = eventList.begin(); (curr != eventList.end()); curr++) {
        muse::Event* const event = *curr;
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sentTime)) {
            event->decreaseReference();   // Canceled event!
            removedCount++;
        } else {
            retained.push_back(event);    // Reschedule the event.
        }
    }
    // Update the sel with list of retained events
    eventList.swap(retained);
    std::make_heap(eventList.begin(), eventList.end(), compare);
    return removedCount;
}
#else
int
HeapEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                           const muse::Time sentTime) {
    UNUSED_PARAM(dest);
    ASSERT(dest != NULL);
    int  numRemoved = 0;
    long currIdx    = eventList.size() - 1;
    // NOTE: Here the heap is sorted based on receive time for
    // scheduling.  However, we are canceling based on sentTime.
    // Consequently, doing any clever optimizations to minimize
    // iterations will backfire!
    while (!eventList.empty() && (currIdx >= 0)) {
        // // // ASSERT(currIdx < eventList.size());
        Event* const evt = eventList[currIdx];
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
            eventList[currIdx] = eventList.back();
            eventList.pop_back();
            EventQueue::fixHeap(eventList, currIdx, compare);
            // Update the current index so that it is within bounds.
            currIdx = std::min<long>(currIdx, eventList.size() - 1);
        } else {
            // Check the previous element in the vector to see if that
            // is a candidate for cancellation.
            currIdx--;
        }
    }
    // Return number of events canceled to track statistics.
    return numRemoved;
}
#endif

void
HeapEventQueue::prettyPrint(std::ostream& os) const {
    os << "HeapEventQueue [size=" << eventList.size() << "]:\n";
    for (auto curr = eventList.begin(); (curr != eventList.end()); curr++) {
        os << " " << (*curr) << std::endl;
    }
    os << std::endl;

}

void
HeapEventQueue::reportStats(std::ostream& os) {
    os << "HeapEventQueue:\n"
       << "\tMax queue size: " << maxQsize << std::endl;
}

muse::Event*
HeapEventQueue::pop_front() {
    ASSERT(!empty());
    std::pop_heap(eventList.begin(), eventList.end(), compare);
    muse::Event* retVal = eventList.back();
    eventList.pop_back();
    return retVal;
}

END_NAMESPACE(muse)

#endif
