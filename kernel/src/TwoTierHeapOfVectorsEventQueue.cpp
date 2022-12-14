#ifndef TWO_TIER_HEAP_OF_VECTORS_EVENT_QUEUE_CPP
#define TWO_TIER_HEAP_OF_VECTORS_EVENT_QUEUE_CPP

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

#include "TwoTierHeapOfVectorsEventQueue.h"
#include "BinaryHeap.h"
#include <algorithm>

BEGIN_NAMESPACE(muse)

// A convenience shortcut used just in this source file
using Tier2List = BinaryHeap<muse::Tier2Entry, muse::EventComp>;

TwoTierHeapOfVectorsEventQueue::TwoTierHeapOfVectorsEventQueue() :
EventQueue("TwoTierHeapOfVectorsEventQueue") {
    // Nothing else to be done.
}

TwoTierHeapOfVectorsEventQueue::~TwoTierHeapOfVectorsEventQueue() {
    // Nothing else to be done.
}

void*
TwoTierHeapOfVectorsEventQueue::addAgent(muse::Agent* agent) {
    agentList.push_back(agent);
    // Create the binary heap that is used to manage events for the agent.
    agent->schedRef.tier2eventPQ = new Tier2List();
    return reinterpret_cast<void*>(agentList.size() - 1);
}

void
TwoTierHeapOfVectorsEventQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    ASSERT(agent->schedRef.tier2eventPQ != NULL);
    // Logically remove events in this agent's tier2 queues/buckets.
    Tier2List& tier2eventPQ = *agent->schedRef.tier2eventPQ;
    for (auto iter = tier2eventPQ.begin(); iter != tier2eventPQ.end(); iter++) {
        for (Event* evt : (*iter).getEventList()) {
            decreaseReference(evt);  // logically remove event
        }
    }
    // clear out tier2 queue (so this agent's time becomes INFINITY).
    tier2eventPQ.clear();
    // Update the heap to place agent with LTSF.
    updateHeap(agent);
}

muse::Event*
TwoTierHeapOfVectorsEventQueue::front() {
 return (!top()->schedRef.tier2eventPQ->empty()) ?
            top()->schedRef.tier2eventPQ->top().getEvent() : NULL;               
}

void
TwoTierHeapOfVectorsEventQueue::pop_front(muse::Agent* agent) {
    // Decrease reference count for all events in the front of the
    // agent event queue before the list of events is removed from the
    // event queue.
    EventContainer& eventList = 
            agent->schedRef.tier2eventPQ->top().getEventList();
    for (Event* evt: eventList) {
        decreaseReference(evt);  // Call base class static method
    }
    agent->schedRef.tier2eventPQ->pop();
}

void
TwoTierHeapOfVectorsEventQueue::getNextEvents(Agent* agent,
                                       EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(agent->schedRef.tier2eventPQ->top().getEvent() != NULL);
    // All events in tier2eventPQ top should have same receive times.
    // The following code is enabled only when assertions are enabled.
    WHEN_ASSERT(const Time eventTime =
                agent->schedRef.tier2eventPQ->getTopTime());
    // Move all the events out of the tier2eventPQ top into the return
    // container.
    container = std::move(agent->schedRef.tier2eventPQ->top().getEventList());
    // Do validation checks on the events in tier2eventPQ.
    for (const Event* event : container) {
         //  All events must have the same receive time.
        ASSERT( event->getReceiveTime() == eventTime );
                // We should never process an anti-message.
        if (event->isAntiMessage()) {
            std::cerr << "Anti-message Processing: " << *event << std::endl;
            std::cerr << "Trying to process an anti-message event, "
                      << "please notify MUSE developers of this issue"
                      << std::endl;
            abort();
        }
        // Ensure that the top event is greater than LVT.
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
    // Finally it is safe to remove this tier2event entry from the event queue,
    // as it's list of events have been added to the inputQueue.
    pop_front(agent);
    // Track bucket/block size statistics
    avgSchedBktSize += container.size();
}

void
TwoTierHeapOfVectorsEventQueue::dequeueNextAgentEvents(muse::EventContainer&
                                                       events) {
    if (!empty()) {
        // Get agent and validate.
        muse::Agent* const agent = top();
        ASSERT(agent != NULL);
        ASSERT(getIndex(agent) == 0);
        // Have the events give up its next set of events.
        getNextEvents(agent, events);
        ASSERT(!events.empty());
        // Fix the position of this agent in the scheduler's heap.
        updateHeap(agent);
    }
}

void
TwoTierHeapOfVectorsEventQueue::enqueue(muse::Agent* agent,
                                        muse::Event* event) {
    // Use helper method (just below this one) to add event and fix-up
    // the queue.  First Increase event reference count for every
    // event added to the event queue.
    increaseReference(event);  // use base class static method
    enqueue(agent, event, true);
}

void
TwoTierHeapOfVectorsEventQueue::enqueue(muse::Agent* agent, muse::Event* event,
                                        const bool fixHeap) {
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    ASSERT(getIndex(agent) < agentList.size());
    // Use linear search O(n) to find match or insert position
    agentBktCount += agent->schedRef.tier2eventPQ->size();
    std::vector<Tier2Entry>::iterator iter;
    Tier2Entry tier2Entry(event);
    iter = agent->schedRef.tier2eventPQ->find(tier2Entry);
    /* If there is an event with a matching receive time in the heap,
    then add the event to the bucket of events associated with
    that particular Tier2Entry object. */ 
    if(iter != agent->schedRef.tier2eventPQ->end()) {
        iter->updateContainer(event);        
    } else {
        /*If there is no event with a matching receive time in the heap,
        then send an instance of Tier2Entry to the binary heap. */ 
        agent->schedRef.tier2eventPQ->push(tier2Entry);
    }
    // Fix the position of this agent in the scheduler's heap.
    if(fixHeap) {
        updateHeap(agent);
    }
}

void
TwoTierHeapOfVectorsEventQueue::enqueue(muse::Agent* agent,
                                        muse::EventContainer& events) {
    ASSERT(agent != NULL);
    ASSERT(getIndex(agent) < agentList.size());
    for(muse::Event* event : events) {
        // Enqueue event but don't waste time fixing-up heap yet for
        // this agent.  We will do it at the end after all events are
        // added.  However, we don't increase reference counts in this
        // API.
        enqueue(agent, event, false);
    }
    // Clear out all the events in the incoming container.
    events.clear();
    // Update the location of this agent on the heap as needed.
    updateHeap(agent);
}
  
int
TwoTierHeapOfVectorsEventQueue::eraseAfter(muse::Agent* dest,
                                           const muse::AgentID sender,
                                           const muse::Time sentTime) {
    ASSERT(dest->schedRef.tier2eventPQ != NULL);
    int  numRemoved =0;
    Tier2List& tier2eventPQ = *dest->schedRef.tier2eventPQ;
    long currIdx = tier2eventPQ.size() - 1;
    auto iter = tier2eventPQ.rbegin();
    while ( (iter != tier2eventPQ.rend()) && currIdx >= 0 ) {
        if(iter->getReceiveTime() > sentTime) {
            EventContainer& eventList = iter->getEventList();
            size_t index = 0;
            while (!eventList.empty() && (index < eventList.size())) {
                Event* const evt = eventList[index];
                ASSERT(evt != NULL);
                if (isFutureEvent(sender, sentTime, evt)) {
                    decreaseReference(evt);  // use base class static method
                    numRemoved++;
                    eventList[index] = eventList.back();
                    eventList.pop_back();
                } else {
                    index++;  // onto next event in this bucket
                }
            }
            // If all events are canceled then this bucket needs to be
            // removed from the agent's event queue.
            if (eventList.empty()) {
                tier2eventPQ.remove(currIdx);
            }
        }
        currIdx--;
        iter++;
    }
    // Update the 1st tier heap for scheduling.
    updateHeap(dest);
    // Return number of events canceled to track statistics.
    return numRemoved;
}

void
TwoTierHeapOfVectorsEventQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);
    const long comps = std::log2(agentList.size()) *
        avgSchedBktSize.getCount() + fixHeapSwapCount.getSum();
    os << "Average #buckets per agent   : " << agentBktCount    << std::endl;
    os << "Average scheduled bucket size: " << avgSchedBktSize << std::endl;
    os << "Average fixHeap compares     : " << fixHeapSwapCount << std::endl;
    os << "Compare estimate             : " << comps            << std::endl;
}

void
TwoTierHeapOfVectorsEventQueue::prettyPrint(std::ostream& os) const {
    os << "TwoTierHeapOfVectorsEventQueue::prettyPrint() : not implemented.\n";
}

size_t
TwoTierHeapOfVectorsEventQueue::getIndex(muse::Agent *agent) const {
    ASSERT(agent != NULL);
    size_t index = reinterpret_cast<size_t>(agent->fibHeapPtr);
    ASSERT(index < agentList.size());
    ASSERT(agentList[index] == agent);
    return index;
}

size_t
TwoTierHeapOfVectorsEventQueue::updateHeap(muse::Agent* agent) {
    ASSERT(agent != NULL);
    size_t index = getIndex(agent);
    if (agent->oldTopTime != getTopTime(agent)) {
        index = fixHeap(index);
        // Update the position of the agent in the scheduler's heap
        // Validate.
        ASSERT(agentList[index] == agent);
        ASSERT(getIndex(agent) == index);
        // Update time value as well for future access.
        agent->oldTopTime = getTopTime(agent);
        // Validation check.
        ASSERT(getTopTime(agentList[0]) <= getTopTime(agentList[1]));
    }
    // Return the new index position of the agent.
    return index;
}

size_t
TwoTierHeapOfVectorsEventQueue::fixHeap(size_t currPos) {
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
