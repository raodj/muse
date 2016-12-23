#ifndef HEAP_OF_VECTORS_EVENT_QUEUE_CPP
#define HEAP_OF_VECTORS_EVENT_QUEUE_CPP

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

#include "HeapOfVectorsEventQueue.h"
#include <algorithm>

BEGIN_NAMESPACE(muse)

// A convenience shortcut used just in this source file
using Tier2List = std::vector<Tier2Entry>;

HeapOfVectorsEventQueue::HeapOfVectorsEventQueue() :
    EventQueue("HeapOfVectorsEventQueue") {
    // Nothing else to be done.
}

HeapOfVectorsEventQueue::~HeapOfVectorsEventQueue() {
    // Nothing else to be done.
}

void*
HeapOfVectorsEventQueue::addAgent(muse::Agent* agent) {
    agentList.push_back(agent);
    // Create the vector that is used to manage events for the agent.
    agent->tier2 = new Tier2List();
    return reinterpret_cast<void*>(agentList.size() - 1);
}

void
HeapOfVectorsEventQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    ASSERT(!empty());
    // Decrease reference count for all events in the agent event queue
    // before agent removal.
    ASSERT( agent->tier2 != NULL );
    // Logically remove events in this agent's tier2 queues/buckets
    Tier2List& tier2eventPQ = *agent->tier2;
    for (muse::Tier2Entry& bucket : tier2eventPQ) {
        for (Event* evt : bucket.getEventList()) {
            evt->decreaseReference();  // logically remove event
        }
    }
    // Clear out tier2 queue (so this agent's time becomes PINFINITY)
    agent->tier2->clear();
    // Update the heap to place agent with LTSF
    updateHeap(agent);
}

muse::Event*
HeapOfVectorsEventQueue::front() {
    return (!top()->tier2->empty()) ? top()->tier2->front().getEvent() : NULL;
}

void
HeapOfVectorsEventQueue::pop_front(muse::Agent* agent) {
    // Decrease reference count for all events in the front of the
    // agent event queue before the list of events is removed from the
    // event queue.
    EventContainer& eventList = agent->tier2->front().getEventList();
    for (Event* evt: eventList) {
        evt->decreaseReference();
    }
    agent->tier2->erase(agent->tier2->begin()); 
}

void
HeapOfVectorsEventQueue::getNextEvents(Agent* agent,
                                       EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(agent->tier2->front().getEvent() != NULL);
    // All events in tier2 front should have same receive times
    const muse::Time eventTime = agent->tier2->front().getReceiveTime();
    // Move all the events out of the tier2 front into the return contianer
    container = std::move(agent->tier2->front().getEventList());
    // Do validation checks on the events in tier2
    for (const Event* event : container) {
        //  All events must have the same receive time
        ASSERT( event->getReceiveTime() == eventTime );
        // We should never process an anti-message.
        if (event->isAntiMessage()) {
            std::cerr << "Anti-message Processing: " << *event << std::endl;
            std::cerr << "Trying to process an anti-message event, "
                      << "please notify MUSE developers of this issue"
                      << std::endl;
            abort();
        }
        // Ensure that the top event is greater than LVT
        if (event->getReceiveTime() <= agent->getTime(Agent::LVT)) {
            std::cerr << "Agent is being scheduled to process an event ("
                      << *event << ") that is at or below it LVT (LVT="
                      << agent->getTime(Agent::LVT) << ", GVT="
                      << agent->getTime(Agent::GVT)
                      << "). This is a serious error. Aborting.\n";
            std::cerr << *agent << std::endl;
            abort();
        }
        // Ensure reference counts are consistent.
        ASSERT(event->getReferenceCount() < 3);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    }
    // Remove the entry at the beginning of the queue.
    pop_front(agent);
    // Track bucket/block size statistics
    avgSchedBktSize += container.size();
}

void
HeapOfVectorsEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (!empty()) {
        // Get agent and validate.
        muse::Agent* const agent = top();
        ASSERT(agent != NULL);
        ASSERT(getIndex(agent) == 0);
        // Have the events give up its next set of events
        getNextEvents(agent, events);
        ASSERT(!events.empty());
        // Fix the position of this agent in the scheduler's heap.
        updateHeap(agent);
    }
}

void
HeapOfVectorsEventQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    // Use helper method (just below this one) to add event and fix-up
    // the queue.  First Increase event reference count for every
    // event added to the event queue.
    event->increaseReference();
    enqueue(agent, event, true);
}

void
HeapOfVectorsEventQueue::enqueue(muse::Agent* agent, muse::Event* event,
                                 const bool fixHeap) {
    ASSERT(agent != NULL);
    ASSERT(event != NULL); 
    ASSERT( agent->tier2 != NULL );   
    ASSERT(getIndex(agent) < agentList.size());
    // A convenience reference to tier2 list of buckets
    Tier2List& tier2 = *agent->tier2;
    // Use binary search O(log n) to find match or insert position
    agentBktCount += tier2.size();
    Tier2List::iterator iter = 
        std::lower_bound(tier2.begin(), tier2.end(), event, lessThan);
    // There are 3 cases: 1. we found matching bucket, 2: iterator
    // to bucket with higher recvTime, or 3: tier2.end().
    if (iter == tier2.end()) {
        tier2.push_back(Tier2Entry(event));  // add new entry to end.
    } else if (iter->getReceiveTime() == event->getReceiveTime()) {
        // We found an existing bucket. Append this event to this
        // existing bucket.
        iter->updateContainer(event);
    } else {
        // If there is no bucket with a matching receive time in Tier2
        // vector, then insert an instance of Tier2Entry (aka bucket)
        // into the vector at the appropriate position.
        ASSERT(iter->getReceiveTime() > event->getReceiveTime());
        agent->tier2->insert(iter, Tier2Entry(event));
    }
    // ASSERT(std::is_sorted(tier2.begin(), tier2.end()));
    // Fix the position of this agent in the scheduler's heap.
    if (fixHeap) {
        updateHeap(agent);
    }
}

void
HeapOfVectorsEventQueue::enqueue(muse::Agent* agent,
                                 muse::EventContainer& events) {
    ASSERT(agent != NULL);
    // Note: events container may be empty!
    ASSERT(getIndex(agent) < agentList.size());
    // Add all events to tier2 entries appropriately. 
    for (muse::Event* event : events) {
        // Enqueue event but don't waste time fixing-up heap yet for
        // this agent.  We will do it at the end after all events are
        // added.  However, we don't increase reference counts in this
        // API.
        enqueue(agent, event, false);
    }
    // Clear out all the events in the incoming container
    events.clear();
    // Update the location of this agent on the heap as needed.
    updateHeap(agent);
}

int
HeapOfVectorsEventQueue::eraseAfter(muse::Agent* dest,
                                    const muse::AgentID sender,
                                    const muse::Time sentTime) {
    int  numRemoved = 0;
    ASSERT( dest->tier2 != NULL );
    Tier2List& tier2eventPQ = *dest->tier2;
    long currIdx = tier2eventPQ.size() - 1;
    while (!tier2eventPQ.empty() && (currIdx >= 0)) {
        if (tier2eventPQ[currIdx].getReceiveTime() > sentTime) {
            EventContainer& eventList = tier2eventPQ[currIdx].getEventList();
            size_t index = 0;
            while (!eventList.empty() && (index < eventList.size())) {
                Event* const evt = eventList[index];
                ASSERT(evt != NULL);
                if (isFutureEvent(sender, sentTime, evt)) {
                    evt->decreaseReference();
                    numRemoved++;
                    eventList[index] = eventList.back();
                    eventList.pop_back();
                } else {
                    index++;  // onto next event in this bucket
                }
            }
            // If all events are canceled then this bucket needs to be
            // removed from the tier2 entry.
            if (eventList.empty()) {
                tier2eventPQ.erase(tier2eventPQ.begin() + currIdx);
            }
        }
        currIdx--;
    }
    // Update the 1st tier heap for scheduling.
    updateHeap(dest);
    // Return number of events canceled to track statistics.
    return numRemoved;
}

void
HeapOfVectorsEventQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);
    os << "Average #buckets per agent   : " << agentBktCount    << std::endl;
    os << "Average scheduled bucket size: " << avgSchedBktSize  << std::endl;
    os << "Average fixHeap compares     : " << fixHeapSwapCount << std::endl;
}

void
HeapOfVectorsEventQueue::prettyPrint(std::ostream& os) const {
    os << "HeapOfVectorsEventQueue::prettyPrint() : not implemented.\n";  
}

size_t
HeapOfVectorsEventQueue::getIndex(muse::Agent *agent) const {
    ASSERT(agent != NULL);
    size_t index = reinterpret_cast<size_t>(agent->fibHeapPtr);
    ASSERT(index < agentList.size());
    ASSERT(agentList[index] == agent);
    return index;
}

size_t
HeapOfVectorsEventQueue::updateHeap(muse::Agent* agent) {
    ASSERT(agent != NULL);
    size_t index = getIndex(agent);
    if (agent->oldTopTime != getTopTime(agent)) {
        index = fixHeap(index);
        // Update the position of the agent in the scheduler's heap
        // Validate
        ASSERT(agentList[index] == agent);
        ASSERT(getIndex(agent) == index);
        // Update time value as well for future access
        agent->oldTopTime = getTopTime(agent);
        // Validation check.
        ASSERT(getTopTime(agentList[0]) <= getTopTime(agentList[1]));
    }
    // Return the new index position of the agent
    return index;
}

size_t
HeapOfVectorsEventQueue::fixHeap(size_t currPos) {
    ASSERT(currPos < agentList.size());
    muse::Agent* value    = agentList[currPos];
    const size_t len      = (agentList.size() - 1) / 2;
    size_t secondChild    = currPos;
    int          opCount  = 0;
    // This code was borrowed from libstdc++ implementation to ensure
    // that the fix-ups are consistent with std::make_heap API.
    while (secondChild < len)  {
        secondChild = 2 * (secondChild + 1);
        if (compare(agentList[secondChild], agentList[secondChild - 1])) {
            secondChild--;
        }
        agentList[currPos] = std::move(agentList[secondChild]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*>(currPos);
        currPos = secondChild;
        opCount++;  // track statistics on number of operations performed
    }
    if (((agentList.size() & 1) == 0) &&
        (secondChild == (agentList.size() - 2) / 2)) {
        secondChild        = 2 * (secondChild + 1);
        agentList[currPos] = std::move(agentList[secondChild - 1]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*>(currPos);
        currPos            = secondChild - 1;
        opCount++;  // track statistics on number of operations performed
    }
    // Use libstdc++'s internal method to fix-up the vector from the
    // given location.
    // std::__push_heap(agentList.begin(), currPos, 0, value,
    //                 __gnu_cxx::__ops::__iter_comp_val(compare));
    
    size_t parent = (currPos - 1) / 2;
    while ((currPos > 0) && (compare(agentList[parent],value))) {
        agentList[currPos] = std::move(agentList[parent]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*>(currPos);
        currPos = parent;
        parent  = (currPos - 1) / 2;
        opCount++;  // track statistics on number of operations performed
    }
    agentList[currPos] = value;
    agentList[currPos]->fibHeapPtr = reinterpret_cast<void*>(currPos);
    // Update aggregate statistics
    fixHeapSwapCount += opCount;
    // Return the final index position for the agent
    return currPos;
}

END_NAMESPACE(muse)

#endif
