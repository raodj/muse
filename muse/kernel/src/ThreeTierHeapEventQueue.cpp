#ifndef THREE_TIER_HEAP_EVENT_QUEUE_CPP
#define THREE_TIER_HEAP_EVENT_QUEUE_CPP

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

#include "ThreeTierHeapEventQueue.h"
#include "BinaryHeap.h"
#include <algorithm>

BEGIN_NAMESPACE(muse)

ThreeTierHeapEventQueue::ThreeTierHeapEventQueue() :
EventQueue("ThreeTierHeapEventQueue") {
    // Nothing else to be done.
}

ThreeTierHeapEventQueue::~ThreeTierHeapEventQueue() {
    // Nothing else to be done.
}

void*
ThreeTierHeapEventQueue::addAgent(muse::Agent* agent) {
    agentList.push_back(agent);
    return reinterpret_cast<void*>(agentList.size() - 1);
}

muse::Event*
ThreeTierHeapEventQueue::front() {
    muse::Event* retVal = NULL;
    if (!empty()) {
        retVal = top()->eventPQ->top();
    }
    return retVal;
}

void
ThreeTierHeapEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (!empty()) {
        // Get agent and validate.
        muse::Agent* const agent = top();
        ASSERT(getIndex(agent) == 0);
        // Have the events give up its next set of events
        agent->getNextEvents(events);
        ASSERT(!events.empty());
        // Fix the position of this agent in the scheduler's heap
        updateHeap(agent);
    }
}

void
ThreeTierHeapEventQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    ASSERT(getIndex(agent) < agentList.size());
    /*If the heap is empty, then create and send an instance of Tier2Entry
      to the binary heap.
     */
    if(agent->myEventPQ->empty()) {
        Tier2Entry tier2Entry(event);
        agent->myEventPQ->push(tier2Entry);
    }else {
        /*If the heap is not empty, then compare the event's receive time
          against the receive time of the events in the heap.
         */
        size_t index = 0;
        size_t len = agent->myEventPQ->size() - 1;
        Tier2Entry* currIdx = &agent->myEventPQ->top();
        Tier2Entry tier2Entry(event);
        while(index <= len) {
            /*If there is an event with a matching receive time in the heap,
              then add the event to the list of events associated with
              that particular Tier2Entry object */
            if((currIdx+index)->getRecvTime() == event->getReceiveTime()) {
                agent->myEventPQ->top().updateContainer(event);
                break;
            }
            /*If there is no event with a matching receive time in the heap,
             then send an instance of Tier2Entry to the binary heap. 
             */
            else if((currIdx+index)->getRecvTime() != event->getReceiveTime() &&
                    (index == len)) {
                agent->myEventPQ->push(tier2Entry);
            }
            index++;
        }
    }
    updateHeap(agent);
}

void
ThreeTierHeapEventQueue::enqueue(muse::Agent* agent,
                               muse::EventContainer& events) {
    ASSERT(agent != NULL);
    ASSERT(!events.empty());
    ASSERT(getIndex(agent) < agentList.size());
    EventContainer::iterator iter = events.begin();
    // Compare the list of events in the EventContainer with the event
    // receive times on the heap.
    while(iter!=events.end()) {
        muse::Event* event = *iter;
        size_t index = 0;
        size_t len = agent->myEventPQ->size() - 1;
        Tier2Entry* currIdx = &agent->myEventPQ->top();
        Tier2Entry tier2Entry(event);
        while(index <= len) {
            /*If there is a match in the receive time, then append the event 
              to the list of events associated with that particular
              Tier2Entry object.*/
            if((currIdx+index)->getRecvTime() == event->getReceiveTime()) {
                agent->myEventPQ->top().updateContainer(event);
                break;
            }
            /*If there is no match, then create a Tier2Entry object and add
              the object to the vector of Tier2Entry objects.*/
            else if((currIdx+index)->getRecvTime() != event->getReceiveTime() &&
                    (index == len)) {
                tier2.push_back(tier2Entry);
            }
            index++;
        }
        iter++;  
    }
    agent->myEventPQ->push(tier2);
    tier2.clear();
    updateHeap(agent);
}

int
ThreeTierHeapEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                                  const muse::Time sentTime) {
    ASSERT(dest != NULL);
    ASSERT(getIndex(dest) < agentList.size());
    // Get agent's heap to cancel out events.
    int numRemoved = dest->eventPQ->removeFutureEvents(sender, sentTime);
    // Update the 2nd tier heap for scheduling.
    updateHeap(dest);
    return numRemoved;
}

void
ThreeTierHeapEventQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);
    // No statistics are currently reported.
}

void
ThreeTierHeapEventQueue::prettyPrint(std::ostream& os) const {
    os << "ThreeTierHeapEventQueue::prettyPrint() : not implemented.\n";
}

size_t
ThreeTierHeapEventQueue::getIndex(muse::Agent *agent) const {
    ASSERT(agent != NULL);
    size_t index = reinterpret_cast<size_t>(agent->fibHeapPtr);
    ASSERT(index < agentList.size());
    ASSERT(agentList[index] == agent);
    return index;
}

size_t
ThreeTierHeapEventQueue::updateHeap(muse::Agent* agent) {
    ASSERT(agent != NULL);
    size_t index = getIndex(agent);
    if (agent->oldTopTime != agent->getTopTime()) {
        index = fixHeap(index);
        // Update the position of the agent in the scheduler's heap
        // Validate
        ASSERT(agentList[index] == agent);
        ASSERT(getIndex(agent) == index);
        // Update time value as well for future access
        agent->oldTopTime = agent->getTopTime();
        // Validation check.
        ASSERT(agentList[0]->getTopTime() <= agentList[1]->getTopTime());
    }
    // Return the new index position of the agent
    return index;
}

size_t
ThreeTierHeapEventQueue::fixHeap(size_t currPos) {
    ASSERT(currPos < agentList.size());
    muse::Agent* value    = agentList[currPos];
    const size_t len      = (agentList.size() - 1) / 2;
    size_t secondChild    = currPos;
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
    } 
    if (((agentList.size() & 1) == 0) &&
        (secondChild == (agentList.size() - 2) / 2)) {
        secondChild        = 2 * (secondChild + 1);
        agentList[currPos] = std::move(agentList[secondChild - 1]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*>(currPos);
        currPos            = secondChild - 1;
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
    // Return the final index position for the agent
    return currPos;    
}

END_NAMESPACE(muse)

#endif
