#ifndef THREE_TIER_SKIP_MT_QUEUE_CPP
#define THREE_TIER_SKIP_MT_QUEUE_CPP

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

#include "ThreeTierSkipMTQueue.h"
#include "Agent.h"
#include <algorithm>

BEGIN_NAMESPACE(muse)

// (deperomm): So so so bad... has to be a better way to do this
template<>
ThreeTierSkipMTQueue::AgentKey 
ThreeTierSkipMTQueue::AgentList::keyMax = 
        std::pair<muse::Time, muse::AgentID>(10000000,10000000);
template<>
ThreeTierSkipMTQueue::AgentKey 
ThreeTierSkipMTQueue::AgentList::keyMaxMinusOne = 
        std::pair<muse::Time, muse::AgentID>(9999999,10000000);
template<>
ThreeTierSkipMTQueue::AgentKey 
ThreeTierSkipMTQueue::AgentList::keyMin = 
        std::pair<muse::Time, muse::AgentID>(0,0);
template<>
muse::Time ThreeTierSkipMTQueue::Tier2ListMT::keyMax         = 10000000;
template<>
muse::Time ThreeTierSkipMTQueue::Tier2ListMT::keyMaxMinusOne =  9999999;
template<>
muse::Time ThreeTierSkipMTQueue::Tier2ListMT::keyMin         =        0;

ThreeTierSkipMTQueue::ThreeTierSkipMTQueue() :
    EventQueueMT("ThreeTierSkipListMTQueue") {
    agentList = new AgentList;
    // Nothing else to be done.
}

ThreeTierSkipMTQueue::~ThreeTierSkipMTQueue() {
    // Clear up memory allocated for HOETier2Entry
    for (HOETier2EntryMT* entry : tier2Recycler) {
        delete entry;
    }
    
    delete agentList;
}

void*
ThreeTierSkipMTQueue::addAgent(muse::Agent* agent) {
    // Create the vector that is used to manage events for the agent.
    agent->tier2MT = new Tier2ListMT;
    
    agent->key.first  = 10; // (deperomm): testing, make this keyMax later
    agent->key.second = agent->getAgentID();
    
    // insert this agent at the back of the queue
    // (deperomm): hacky attempt to ensure greater than any other time value
    // had issues with KEY_MAX-1 < KEY_MAX
    muse::Agent* dup = agentList->insert(agent->key, agent);
    ASSERT(dup == NULL); // make sure no duplicates
    
    // the return is for the fibHeapPointer, which this queue doesn't use
    return NULL;
}

void
ThreeTierSkipMTQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    // Decrease reference count for all events in the agent event queue
    // before agent removal.
    ASSERT( agent->tier2MT != NULL );
    
    // remove at top level
    void* check = agentList->deleteEntry(agent->key);
    ASSERT(check != NULL);
    
    // todo(deperomm): Fix all of this... implement iterator in LockFreePQ
    
    HOETier2EntryMT *entry;
    Tier2ListMT::Node_t *curr = agent->tier2MT->getNode(.000001); // hack to get front
    Tier2ListMT::Node_t *tail = agent->tier2MT->getTail();
    while (curr != tail) {
        // The next reference may refer to a node that was deleted, if so the
        // value of the node will be null. Since no thread is deleting right
        // now, it is safe to assume that if a node has a value, that value 
        // will be valid until this thread is done with it
        entry = (HOETier2EntryMT *)curr->value;
        if (entry != NULL) {
            // (deperomm) bug in 3tHeap on >= vs > here?...
            std::vector<muse::Event*> eventList = entry->getEventList();
            
            entry->entryGuard.lock();
            ASSERT(!entry->removed); // not possible if we're on dequeue thread
            size_t index = 0;
            while (!eventList.empty() && (index < eventList.size())) {
                Event* const evt = eventList[index];
                ASSERT(evt != NULL);
                decreaseReference(evt);  // Logically free/recycle event
                eventList[index] = eventList.back();
                eventList.pop_back();
            }
            // If all events are canceled then this bucket needs to be
            // removed from the tier2 entry.
            ASSERT (eventList.empty());
            // execute a hacky logical delete inside tier2MT LockFreePQ
            // we're only allowed to do this because no other thread is
            // removing on this queue right now, otherwise could conflict
            ASSERT(curr->value != NULL); // shouldn't be possible...
            curr->value = NULL;
            tier2Recycler.emplace_back(entry);
        }
        curr = curr->next[0];
    }
}

//muse::Event*
//ThreeTierHeapEventQueue::front() {
//    return (!top()->tier2->empty()) ? top()->tier2->front()->getEvent() : NULL;
//}

Agent* 
ThreeTierSkipMTQueue::popNextAgent() {
    muse::Agent* agent = agentList->deleteMin();
    ASSERT(agent != NULL); // should only happen if all agents were removed
    return agent;
}

void
ThreeTierSkipMTQueue::dequeueNextEvents(muse::Agent *agent,
                                              muse::EventContainer& events) {
    
    
    ASSERT(events.empty());
    ASSERT(agent->tier2MT != NULL);
    
    // pop the tier2 entry off the tier2 queue
    HOETier2EntryMT *tier2Entry = agent->tier2MT->deleteMin();
    
    if (tier2Entry == NULL) {
        // there are no entries in the tier2 queue, return without modifying
        // container
        return;
    }
    
    ASSERT(tier2Entry->getEvent() != NULL);
    
    // we have exclusive dequeue rights to this tier2Entry. If another thread
    // inserts while we're processing this, that's okay we'll rollback later
    
    // wait for any pending inserts to this 3rd tier queue, and then mark
    // this entry as deleted, that way other threads stop trying to insert 
    // into it and we can safely extract its events
    // @see tryTier2Insert() method.
    tier2Entry->entryGuard.lock();
    tier2Entry->removed = true;
    tier2Entry->entryGuard.unlock();
    
    
    // Copy all the events out of the tier2 front into the return contianer
    std::vector<muse::Event*> evtList = tier2Entry->getEventList();
    events.assign(evtList.begin(), evtList.end());
    
    DEBUG(std::cout << "removed agent " << agent->key.second << " with key " 
          << agent->key.first << ") and events at time " 
          << events.front()->getReceiveTime() << std::endl);
    
    DEBUG({
        // All events in tier2 front should have same receive times
        WHEN_ASSERT(const muse::Time eventTime = tier2Entry->getReceiveTime());
        // Do validation checks on the events in tier2
        for (const Event* event : container) {
            //  All events must have the same receive time
            ASSERT( event->getReceiveTime() == eventTime );
        }
    });
    
//    DEBUG({
//            
//                
//                // We should never process an anti-message.
//                if (event->isAntiMessage()) {
//                    std::cerr << "Anti-message Processing: " << *event
//                              << std::endl;
//                    std::cerr << "Trying to process an anti-message event, "
//                              << "please notify MUSE developers of this issue"
//                              << std::endl;
//                    abort();
//                }
//                // Ensure that the top event is greater than LVT
//                if (event->getReceiveTime() <= agent->getTime(Agent::LVT)) {
//                    std::cerr << "Agent is being scheduled to process "
//                              << "an event ("
//                              << *event << ") that is at or below it LVT (LVT="
//                              << agent->getTime(Agent::LVT) << ", GVT="
//                              << agent->getTime(Agent::GVT)
//                              << "). This is a serious error. Aborting.\n";
//                    std::cerr << *agent << std::endl;
//                    abort();
//                }
//                // Ensure reference counts are consistent.
//                ASSERT(event->getReferenceCount() < 3);
//                DEBUG(std::cout << "Delivering: " << *event << std::endl);
//            }
//        });
    
    // Recycle the entry so it can be reused
    recycleTier2Entry(tier2Entry);
    
    // Track bucket/block size statistics
    avgSchedBktSize += events.size();
    
}

void
ThreeTierSkipMTQueue::pushAgent(muse::Agent* agent) {
    
    // Let any event inserts on this agent finish their restructures
    agent->restructureMutex.lock();
    
    // Get the next key at this moment (could change if another thread is
    // actively inserting right now)
    // This is only safe because we have exclusive access to this agent
    // to dequeue it. Concurrent inserts will only lower our timestamp, not
    // make it higher. If such an insert existed, it would restructure itself
    // once it can get the restructure lock for this agent. This also means
    // that nextMin will be a valid entry timestamp, since it won't get
    // concurrently deleted while we do this restructure.
    // The new key is the next min of the tier2 queue
    // This also only works if rollbacks happen on the dequeue thread, not
    // any enqueue threads, as a rollback could also impact the above comment.
    // basically nothing can be removing from the tier2 Q when nextMin is called
    agent->key.first = agent->tier2MT->nextMin();
    
    DEBUG(std::cout << "putting back agent " << agent->getAgentID() 
          << " with new " << agent->key.first << std::endl);
    
    // Put the agent back in, sorted by the new key.
    // If an insert was happening now, this key could now be wrong. This is OK
    // b/c that insert operation will have to wait until the lock we have
    // becomes available, at which point it will fix any incorrect placement
    muse::Agent *dup = agentList->insert(agent->key, agent);
    ASSERT(dup == NULL); // ensure actually inserted
    
    
    agent->restructureMutex.unlock();
}

void
ThreeTierSkipMTQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    // Use helper method (just below this one) to add event and fix-up
    // the queue.  First Increase event reference count for every
    // event added to the event queue.
    ASSERT( event != NULL );
    ASSERT( event->getReferenceCount() < 2 );
    increaseReference(event);  // Call base class method to increase reference
    enqueueEvent(agent, event);
    
    restructureTopQueue(agent, event->getReceiveTime());
}

void
ThreeTierSkipMTQueue::enqueue(muse::Agent* agent,
                                muse::EventContainer& events) {
    ASSERT(agent != NULL);
    
    // Note: events container may be empty!
    // This API call doesn't increase reference count?
    
    // (deperomm): [events] doesn't come in sorted right? Must find min event
    muse::Event *minEvent;
    muse::Time   minKey = AgentList::keyMax.first; // (deperomm) keyMax issue
    
    // Add all events to tier2 entries appropriately. 
    for (muse::Event* event : events) {
        // We don't increase reference counts in this API.
        ASSERT(event != NULL);
        enqueueEvent(agent, event);
        ASSERT(event->getReceiveTime() < AgentList::keyMax.first);
        if (event->getReceiveTime() < minKey) {
            minEvent = event;
            minKey   = event->getReceiveTime();
        }
    }
    // Clear out all the events in the incoming container
    events.clear();
    
    if (minKey != AgentList::keyMax.first) { // if we inserted something
        ASSERT(minEvent != NULL);
        restructureTopQueue(agent, minEvent->getReceiveTime());
    }
}

void
ThreeTierSkipMTQueue::enqueueEvent(muse::Agent* agent, muse::Event* event) {
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    ASSERT( agent->tier2MT != NULL );
    
    // A convenience reference to tier2 list of buckets
    Tier2ListMT *tier2 = agent->tier2MT;

    // todo(deperomm): agentBktCount stat not being kept, add size to LF queue?
//    agentBktCount += tier2.size(); size not kept on LockFreePQ as of now
    
    // Search the agent's bucket for the entry at this event's timestamp
    HOETier2EntryMT *tier2Entry = tier2->getEntry(event->getReceiveTime());
    
    if (tier2Entry != NULL) {
        // We got an existing entry, but it's possible a thread is dequeuing it
        // concurrently. Attempt to insert the event into the entry, but if
        // it fails it means we missed the window to insert and we must make
        // a new one, inevitably resulting in a rollbackagentList
        if (!tryTier2Insert(tier2Entry, event)) {
            tier2Entry = NULL; // failed to insert, make a new one
        } else {
            DEBUG(std::cout << "Inserted into an existing Tier2Entry" << std::endl);
        }
    }
    
    if (tier2Entry == NULL) {
        // No valid entry at the specified timestamp, make a new one and insert
        
        // checks if a duplicate exists in the queue at the moment we insert
        // if a duplicate does exist at the moment we insert, must try and
        // insert into it, an operation which itself could fail, thus the loop
        HOETier2EntryMT *dup; 
        // loop until successful insert
        do {
            tier2Entry = makeTier2Entry(event);
            dup = tier2->insert(event->getReceiveTime(), tier2Entry);
            if (dup != NULL) {
                // another thread made an entry while we were making it.
                // use this new one, checking to make sure it wasn't also
                // concurrently dequeued with the while loop.
                recycleTier2Entry(tier2Entry); // recycle the one we made
                if (tryTier2Insert(dup, event)) {
                    dup = NULL; // event inserted successfully, exit the loop
                    DEBUG(std::cout << "Inserted into a duplicate Tier2Entry (lost race to make new)" << std::endl);
                }
            } else {
                // successfully inserted the new entry this thread created
                DEBUG(std::cout << "Inserted a new Tier2Entry" << std::endl);
            }
        } while (dup != NULL); // make sure "duplcate entry" value is null
    }
}

void
ThreeTierSkipMTQueue::restructureTopQueue(muse::Agent* agent, muse::Time newTime) {
    // todo(deperomm): switch this to a guard
    agent->restructureMutex.lock();
    
    if (agent->key.first <= newTime) {
        // no need to restructure
        agent->restructureMutex.unlock();
        return;
    }
    
    // try to find the agent, if not found that means it is actively
    // dequeuing, and we can return as it will get our key by checking
    // the tier2 queue after it gets the lock
    muse::Agent *entry = agentList->deleteEntry(agent->key);
    
//    std::cout << "restructure top - agent " << entry->getAgentID() << " had key " << entry->key.first << " new " << newTime << std::endl;
    
    if (entry == NULL) {
        // agent is dequeuing but won't re-insert the agent back into the top
        // queue until we release the lock, so we can be sure that our new
        // event will be reflected in the new key when that thread restructures
        // because it doesn't look for the top event to base it's priority on
        // until after it gets the lock, which we have now
        agent->restructureMutex.unlock();
        return;
    }
    
    ASSERT(entry == agent);
    
    // put the agent back into the queue at the right place
    agent->key.first = newTime;
    muse::Agent *dup = agentList->insert(agent->key, agent);
    ASSERT(dup == NULL);
    
    agent->restructureMutex.unlock();
}

int
ThreeTierSkipMTQueue::eraseAfter(muse::Agent* dest,
                                    const muse::AgentID sender,
                                    const muse::Time sentTime) {
    // NOTE: This must be called on a dequeue only thread w/ no other removals
    // being possible for this agent (dest)
    // This method assumes that there is only one thread at a time that
    // can remove events from an agent at a time, and this is that thread
    // In other words only concurrent inserts should be able to happen right now
    
    // todo(deperomm): Make a better way to iterate queue
    
    int  numRemoved = 0;
    HOETier2EntryMT *entry;
    ASSERT( dest->tier2MT != NULL );
    Tier2ListMT::Node_t *curr = dest->tier2MT->getNode(sentTime);
    Tier2ListMT::Node_t *tail = dest->tier2MT->getTail();
    ASSERT(!is_marked_ref(curr)); // only possible if another thread is dequeing
    while (curr != tail) {
        // The next reference may refer to a node that was deleted, if so the
        // value of the node will be null. Since no thread is deleting right
        // now, it is safe to assume that if a node has a value, that value 
        // will be valid until this thread is done with it
        entry = curr->value;
        if (entry != NULL) {
            // (deperomm) bug in 3tHeap on >= vs > here?...
            ASSERT(entry->getReceiveTime() >= sentTime);
            std::vector<muse::Event*> eventList = entry->getEventList();
            
            entry->entryGuard.lock();
            
            ASSERT(!entry->removed); // not possible if we're on the dequeue thread
            size_t index = 0;
            while (!eventList.empty() && (index < eventList.size())) {
                Event* const evt = eventList[index];
                ASSERT(evt != NULL);
                if (isFutureEvent(sender, sentTime, evt)) {
                    DEBUG(std::cout << "  Cancelling event: " << *evt << std::endl);
                    decreaseReference(evt);  // Logically free/recycle event
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
                // execute a hacky logical delete inside tier2MT LockFreePQ
                // we're only allowed to do this because no other thread is
                // removing on this queue right now, otherwise could conflict
                ASSERT(curr->value != NULL); // shouldn't be possible...
                curr->value = NULL;
                tier2Recycler.emplace_back(entry);
            }
            
            entry->entryGuard.unlock();
        }
        curr = curr->next[0];
        ASSERT(!is_marked_ref(curr)); // sign another thread is dequeing
    }
    // if we deleted events that affected our top tier prioirty, restructure
    // we're allowed to use nextMin because we're the only dequeuing thread
    restructureTopQueue(dest, dest->tier2MT->nextMin());
    // Return number of events canceled to track statistics.
    return numRemoved;
}



void
ThreeTierSkipMTQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);
    // todo(deperomm): finish stats, include size
//    const long comps = std::log2(agentList.size()) *
//        avgSchedBktSize.getCount() + fixHeapSwapCount.getSum();
    os << "Average #buckets per agent   : " << agentBktCount    << std::endl;
    os << "Average scheduled bucket size: " << avgSchedBktSize  << std::endl;
    os << "Average fixHeap compares     : " << fixHeapSwapCount << std::endl;
//    os << "Compare estimate             : " << comps            << std::endl;
}

void
ThreeTierSkipMTQueue::prettyPrint(std::ostream& os) const {
    os << "HeapOfVectorsEventQueue::prettyPrint() : not implemented.\n";  
}



END_NAMESPACE(muse)

#endif
