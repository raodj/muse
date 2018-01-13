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

#include <algorithm>
#include "TwoTierHeapEventQueue.h"

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
    // Create the binary heap adapter that manages events for the agent.
    agent->schedRef.eventPQ = new BinaryHeapWrapper();
    // Return index of agent used to quickly update the heap
    return reinterpret_cast<void*> (agentList.size() - 1);
}

void
TwoTierHeapEventQueue::removeAgent(muse::Agent* agent) {
    ASSERT(agent != NULL);
    ASSERT(agent->schedRef.eventPQ != NULL);
    // Remove all events for this agent from the 2nd tier heap.
    agent->schedRef.eventPQ->clear();
    // Delete custom 2nd tier binary heap wrapper
    delete agent->schedRef.eventPQ;
    // Replace with reference to default/empty one to ease updating
    // position of agent in the heap.
    agent->schedRef.eventPQ = &EmptyBHW;    
    // Update the heap to place agent with LTSF
    updateHeap(agent);
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
TwoTierHeapEventQueue::getNextEvents(Agent* agent, EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(agent->schedRef.eventPQ->top() != NULL ); 
    BinaryHeapWrapper* const eventPQ = agent->schedRef.eventPQ;
    const Time currTime = eventPQ->getTopTime();  
    do {
        Event* event = eventPQ->top();
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

        ASSERT(event->getReferenceCount() < 3);
        
        // We add the top event we popped to the event container
        increaseReference(event);
        container.push_back(event);

        DEBUG(std::cout << "Delivering: " << *event << std::endl);
        
        // Finally it is safe to remove this event from the eventPQ as
        // it has been added to the inputQueue
        eventPQ->pop();
    } while (!empty() && (TIME_EQUALS(eventPQ->getTopTime(), currTime)));  
}

void
TwoTierHeapEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (!empty()) {
        // Get agent and validate.
        muse::Agent * const agent = top();
        ASSERT(agent != NULL);
        ASSERT(getIndex(agent) == 0);
        // Have the events give up its next set of events
        getNextEvents(agent, events);
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
    ASSERT(getIndex(agent) < agentList.size());
    // Add events to the agent's 1nd tier heap (if any)
    if (!events.empty()) {
        agent->schedRef.eventPQ->push(events);
    }
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
    if (agent->oldTopTime != getTopTime(agent)) {
        index = fixHeap(index);
        // Update the position of the agent in the scheduler's heap
        // Validate
        ASSERT(agentList[index] == agent);
        ASSERT(getIndex(agent) == index);
        // Update time value as well for future access
        agent->oldTopTime = getTopTime(agent);
        // Validation check.
        ASSERT((agentList.size() < 2) ||
               (getTopTime(agentList[0]) <= getTopTime(agentList[1])));
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
