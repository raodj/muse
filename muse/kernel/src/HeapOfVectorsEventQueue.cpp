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
    UNUSED_PARAM(agent);
    return NULL;
}

void
HeapOfVectorsEventQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    ASSERT(!empty());
    tier2.clear();
}

muse::Event*
HeapOfVectorsEventQueue::front() {
    return !empty() ? tier2.front().getEvent() : NULL;
}

void
HeapOfVectorsEventQueue::dequeueNextAgentEvents(muse::EventContainer& container) {
    if (!empty()) {
        const Time currTime = tier2.front().getReceiveTime(); 
        EventContainer eventList = tier2.front().getEventList();
        EventContainer::iterator start = eventList.begin(); 
        EventContainer::iterator end = eventList.end(); 
     
        do {
            Event* event = *start;
            // We should never process an anti-message.
            if (event->isAntiMessage()) {
                std::cerr << "Anti-message Processing: " << *event << std::endl;
                std::cerr << "Trying to process an anti-message event, "
                        << "please notify MUSE developers of this issue"
                        << std::endl;
                abort();
            }

            ASSERT(event->getReferenceCount() < 3);

            event->increaseReference();
            container.push_back(event);
            start++;
            
            DEBUG(std::cout << "Delivering: " << *event << std::endl);
        } while (!empty() && (TIME_EQUALS(tier2.front().getReceiveTime(), currTime)) && start!= end);
        tier2.erase(tier2.begin());
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
    UNUSED_PARAM(agent);
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    Tier2Entry tier2Entry(event);
    if(empty()) {
        tier2.push_back(tier2Entry);
    } else {
    std::vector<Tier2Entry>::iterator first = tier2.begin();
    std::vector<Tier2Entry>::iterator last = tier2.end();
    std::vector<Tier2Entry>::iterator iter = find(first, last, tier2Entry);
    /* If there is an event with a matching receive time in the vector,
    then add the event to the list of events associated with
    that particular Tier2Entry object. */ 
        if(*iter == tier2Entry){
            Tier2Entry& cur = *iter;
            cur.updateContainer(event); 
        } else {    
            /*If there is no event with a matching receive time in the vector,
            then insert an instance of Tier2Entry into the vector at the
            appropriate position. */ 
            tier2.insert(iter, tier2Entry);
        }
    }
}

void
HeapOfVectorsEventQueue::enqueue(muse::Agent* agent,
                               muse::EventContainer& events) {
    ASSERT(agent != NULL);
    ASSERT(!events.empty());
    EventContainer::iterator it = events.begin();
    std::vector<Tier2Entry>::iterator iter;
    std::vector<Tier2Entry>::iterator first = tier2.begin();
    std::vector<Tier2Entry>::iterator last = tier2.end();
    tier2.reserve(tier2.size() + events.size() + 1);
    // Compare the list of events in the EventContainer with the event
    // receive times in the vector.
    while(it!=events.end()) {
        muse::Event* event = *it;
        Tier2Entry tier2Entry(event);
        iter = find(first, last, tier2Entry);
        /*If there is a match in the receive time, then append the event 
        to the list of events associated with that particular
        Tier2Entry object. */
        if(*iter == tier2Entry){
            Tier2Entry& cur = *iter;
            cur.updateContainer(event);     
        } else {
            /*If there is no match, then insert the Tier2Entry object into its 
            position in the vector of Tier2Entry objects. */
            tier2.insert(iter, tier2Entry);   
        }
        it++;
    }
}
  
int
HeapOfVectorsEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                                  const muse::Time sentTime) {
    UNUSED_PARAM(dest);
    ASSERT(dest != NULL);
    int numRemoved = 0;
    // TODO: flesh out the implementation of this method.
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

END_NAMESPACE(muse)

#endif
