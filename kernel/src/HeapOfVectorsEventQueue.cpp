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
    agent->tier2 = new std::vector<Tier2Entry>();
    return reinterpret_cast<void*>(agentList.size() - 1);
}

void
HeapOfVectorsEventQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    ASSERT(!empty());
    // Decrease reference count for all events in the agent event queue
    // before agent removal.
    std::vector<Tier2Entry>* tier2eventPQ = agent->tier2;
    std::vector<Tier2Entry>::iterator start = tier2eventPQ->begin();
    std::vector<Tier2Entry>::iterator end = tier2eventPQ->end();
    while(start!=end) {
        EventContainer eventList = (*start).getEventList();
       for(Event* evt: eventList) {
            evt->decreaseReference();
        }
        start++;
    }
    agent->tier2->clear();
    // Update the heap to place agent with LTSF
    updateHeap(agent);
}

muse::Event*
HeapOfVectorsEventQueue::front() {
    return !top()->tier2->empty() ? top()->tier2->front().getEvent() : NULL;
}

void
HeapOfVectorsEventQueue::pop_front(muse::Agent* agent) {
    // Decrease reference count for all events in the front of the agent event queue
    // before the list of events is removed from the event queue.
    EventContainer eventList = agent->tier2->front().getEventList();
    for(Event* evt: eventList) {
        evt->decreaseReference();
    }
    agent->tier2->erase(agent->tier2->begin()); 
}

void
HeapOfVectorsEventQueue::getNextEvents(Agent* agent, EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(agent->tier2->front().getEvent() != NULL);
    const muse::Time currTime = agent->tier2->front().getReceiveTime();
    EventContainer eventList = agent->tier2->front().getEventList();
    EventContainer::iterator start = eventList.begin();
    EventContainer::iterator end = eventList.end();
    do {
        Event* event = *start;
        start++;
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
        // The events in the event list at the front of the tier2 event queue
        // are added to the event container and we increase the reference count
        // for each event added.
        event->increaseReference();
        container.push_back(event);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() &&
            (TIME_EQUALS(agent->tier2->front().getReceiveTime(), currTime)) &&
             start != end);
    pop_front(agent); 
}

void
HeapOfVectorsEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if(!empty()) {
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

std::vector<Tier2Entry>::iterator
HeapOfVectorsEventQueue::find(std::vector<Tier2Entry>::iterator first,
        std::vector<Tier2Entry>::iterator last, const Tier2Entry& tierTwoEntry) {
    typename std::iterator_traits<std::vector<Tier2Entry>::iterator>::difference_type count, step;
    count = std::distance(first, last);
    std::vector<Tier2Entry>::iterator it;
    // Modified implementation of std::binary_search/std::lower_bound in
    // C++ standard library.
    while(count > 0) {
        it = first;
        step = count / 2;
        std::advance(it, step);
        if(*it == tierTwoEntry) {
            return it;
        } else if(*it < tierTwoEntry) {
            count -= step + 1;
            first=++it;
        } else {
            count = step;
        }
    }
    return first;
}

void
HeapOfVectorsEventQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    ASSERT(getIndex(agent) < agentList.size());
    Tier2Entry tier2Entry(event);
    // Increase event reference count for every event added to the event queue.
    event->increaseReference();
    // Queue must contain an object in order for binary search to work correctly.
    if(agent->tier2->empty()) {
        agent->tier2->push_back(tier2Entry);
    } else {
        std::vector<Tier2Entry>::iterator first = agent->tier2->begin();
        std::vector<Tier2Entry>::iterator last = agent->tier2->end();
        std::vector<Tier2Entry>::iterator iter = find(first, last, tier2Entry);
        /* If there is an event with a matching receive time in the vector,
        then add the event to the list of events associated with
        that particular Tier2Entry object. */
        if (*iter == tier2Entry) {
            Tier2Entry& cur = *iter;
            cur.updateContainer(event);
        } else {
            /*If there is no event with a matching receive time in the vector,
            then insert an instance of Tier2Entry into the vector at the
            appropriate position. */
            agent->tier2->insert(iter, tier2Entry);
        }
    }
    // Fix the position of this agent in the scheduler's heap.
    updateHeap(agent);
}

void
HeapOfVectorsEventQueue::enqueue(muse::Agent* agent,
                               muse::EventContainer& events) {
    ASSERT(agent != NULL);
    ASSERT(!events.empty());
    ASSERT(getIndex(agent) < agentList.size());
    EventContainer::iterator it = events.begin();
    std::vector<Tier2Entry>::iterator iter;
    std::vector<Tier2Entry>::iterator first = agent->tier2->begin();
    std::vector<Tier2Entry>::iterator last = agent->tier2->end();
    // Compare the list of events in the EventContainer with the event
    // receive times in the vector.
    while(it!=events.end()) {
        muse::Event* event = *it;
        Tier2Entry tier2Entry(event);
        // Queue must contain an object in order for binary search to work correctly.
        if(agent->tier2->empty()) {
            agent->tier2->push_back(tier2Entry);
        } else {
            iter = find(first, last, tier2Entry);
            /*If there is a match in the receive time, then append the event 
            to the list of events associated with that particular
            Tier2Entry object. */
            if (*iter == tier2Entry) {
                Tier2Entry& cur = *iter;
                cur.updateContainer(event);
            } else {
                /*If there is no match, then insert the Tier2Entry object into its 
                position in the vector of Tier2Entry objects. */
                agent->tier2->insert(iter, tier2Entry);
            }
        }
        it++;
    }
    events.clear();
    updateHeap(agent);
}

bool
HeapOfVectorsEventQueue::isFutureEvent(const muse::AgentID sender,
        const muse::Time sentTime, const muse::Event* evt) {
    return ((evt->getSenderAgentID() == sender)
                    && (evt->getSentTime() >= sentTime));
}

int
HeapOfVectorsEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                                  const muse::Time sentTime) {
    int  numRemoved = 0;
    std::vector<Tier2Entry>* tier2eventPQ = dest->tier2;
    long currIdx = tier2eventPQ->size() - 1;
    while(!(tier2eventPQ->empty()) && (currIdx >= 0)) {
        if((*tier2eventPQ)[currIdx].getReceiveTime() > sentTime) {
            EventContainer eventList = (*tier2eventPQ)[currIdx].getEventList();
            long index = 0;
            while(!eventList.empty() && (index < eventList.size())) {
                Event* const evt = eventList[index];
                ASSERT(evt != NULL);
                if(isFutureEvent(sender, sentTime, evt)) {
                    evt->decreaseReference();
                    numRemoved++;
                    eventList[index] = eventList.back();
                    eventList.pop_back();
                }
                index++;
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
    // No statistics are currently reported.
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
