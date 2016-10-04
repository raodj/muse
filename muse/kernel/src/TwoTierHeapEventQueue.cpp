#ifndef TWO_TIER_HEAP_EVENT_QUEUE_CPP
#define TWO_TIER_HEAP_EVENT_QUEUE_CPP

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

#include "TwoTierHeapEventQueue.h"
#include <algorithm>

BEGIN_NAMESPACE(muse)

TwoTierHeapEventQueue::TwoTierHeapEventQueue() :
EventQueue("TwoTierHeapEventQueue") {
    // Nothing else to be done.
}

TwoTierHeapEventQueue::~TwoTierHeapEventQueue() {
    // Nothing else to be done.
}

void*
TwoTierHeapEventQueue::addAgent(muse::Agent* agent) {
    agentList.push_back(agent);
    return reinterpret_cast<void*> (agentList.size() - 1);
}

muse::Event*
TwoTierHeapEventQueue::front() {
    muse::Event* retVal = NULL;
    if (!empty()) {
        retVal = top()->schedRef.eventPQ->top();
    }
    return retVal;
}

void
TwoTierHeapEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (!empty()) {
        // Get agent and validate.
        muse::Agent * const agent = top();
        ASSERT(getIndex(agent) == 0);
        // Have the events give up its next set of events
        agent->getNextEvents(events);
        ASSERT(!events.empty());
        // Fix the position of this agent in the scheduler's heap
        updateHeap(agent);
    }
}

void
TwoTierHeapEventQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    ASSERT(getIndex(agent) < agentList.size());
    // Add event to the agent's heap first.
    agent->schedRef.eventPQ->push(event);
    // Now update the position of the agent in this tier for scheduling.
    updateHeap(agent);
}

void
TwoTierHeapEventQueue::enqueue(muse::Agent* agent,
        muse::EventContainer& events) {
    ASSERT(agent != NULL);
    ASSERT(!events.empty());
    ASSERT(getIndex(agent) < agentList.size());
    // Add events to the agent's 1nd tier heap
    agent->schedRef.eventPQ->push(events);
    // Update the 2nd tier heap for scheduling.
    updateHeap(agent);
}

int
TwoTierHeapEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
        const muse::Time sentTime) {
    ASSERT(dest != NULL);
    ASSERT(getIndex(dest) < agentList.size());
    // Get agent's heap to cancel out events.
    int numRemoved = dest->schedRef.eventPQ->removeFutureEvents(sender, sentTime);
    // Update the 2nd tier heap for scheduling.
    updateHeap(dest);
    return numRemoved;
}

void
TwoTierHeapEventQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);
    // No statistics are currently reported.
}

void
TwoTierHeapEventQueue::prettyPrint(std::ostream& os) const {
    os << "TwoTierHeapEventQueue::prettyPreint() : not implemented.\n";
}

size_t
TwoTierHeapEventQueue::getIndex(muse::Agent *agent) const {
    ASSERT(agent != NULL);
    size_t index = reinterpret_cast<size_t> (agent->fibHeapPtr);
    ASSERT(index < agentList.size());
    ASSERT(agentList[index] == agent);
    return index;
}

size_t
TwoTierHeapEventQueue::updateHeap(muse::Agent* agent) {
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
TwoTierHeapEventQueue::fixHeap(size_t currPos) {
    ASSERT(currPos < agentList.size());
    muse::Agent* value = agentList[currPos];
    const size_t len = (agentList.size() - 1) / 2;
    size_t secondChild = currPos;
    // This code was borrowed from libstdc++ implementation to ensure
    // that the fix-ups are consistent with std::make_heap API.
    while (secondChild < len) {
        secondChild = 2 * (secondChild + 1);
        if (compare(agentList[secondChild], agentList[secondChild - 1])) {
            secondChild--;
        }
        agentList[currPos] = std::move(agentList[secondChild]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*> (currPos);
        currPos = secondChild;
    }
    if (((agentList.size() & 1) == 0) &&
            (secondChild == (agentList.size() - 2) / 2)) {
        secondChild = 2 * (secondChild + 1);
        agentList[currPos] = std::move(agentList[secondChild - 1]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*> (currPos);
        currPos = secondChild - 1;
    }
    // Use libstdc++'s internal method to fix-up the vector from the
    // given location.
    // std::__push_heap(agentList.begin(), currPos, 0, value,
    //                 __gnu_cxx::__ops::__iter_comp_val(compare));

    size_t parent = (currPos - 1) / 2;
    while ((currPos > 0) && (compare(agentList[parent], value))) {
        agentList[currPos] = std::move(agentList[parent]);
        agentList[currPos]->fibHeapPtr = reinterpret_cast<void*> (currPos);
        currPos = parent;
        parent = (currPos - 1) / 2;
    }
    agentList[currPos] = value;
    agentList[currPos]->fibHeapPtr = reinterpret_cast<void*> (currPos);
    // Return the final index position for the agent
    return currPos;
}


END_NAMESPACE(muse)

#endif
