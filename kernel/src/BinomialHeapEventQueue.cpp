#ifndef BINOMIAL_HEAP_EVENT_QUEUE_CPP
#define BINOMIAL_HEAP_EVENT_QUEUE_CPP

#include "BinomialHeapEventQueue.h"

BEGIN_NAMESPACE(muse)

BinomialHeapEventQueue::BinomialHeapEventQueue() : EventQueue("HeapEventQueue"), maxQsize(0) {
    // Nothing else to be done.
}

BinomialHeapEventQueue::~BinomialHeapEventQueue() {
    // Nothing else to be done.
}

void*
BinomialHeapEventQueue::addAgent(muse::Agent* agent) {
    UNUSED_PARAM(agent);
    return NULL;
}

void
BinomialHeapEventQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    const AgentID id = agent->getAgentID();
     for (auto iter = binomialHeap.begin();
             iter != binomialHeap.end(); iter++) {
        Event* const evt = *iter;
        ASSERT(evt != NULL);
        // the antiMessage's and if the event is from same sender
        if (evt->getReceiverAgentID() == id) {
            evt->decreaseReference();
            // Identify handle for current object on heap
            auto handle = BinomHeap::s_handle_from_iterator(iter);
            // Remove the event from the binomial heap
            binomialHeap.erase(handle);
        }
     }
}

muse::Event*
BinomialHeapEventQueue::front() {
    return !empty() ? binomialHeap.top() : NULL;
}

muse::Event*
BinomialHeapEventQueue::pop_front() {
    ASSERT(!empty());
    muse::Event* retVal = binomialHeap.top();
    binomialHeap.pop();
    return retVal;
}

void
BinomialHeapEventQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (empty()) {
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
BinomialHeapEventQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    UNUSED_PARAM(agent);
    ASSERT(agent != NULL);
    ASSERT(event != NULL);
    binomialHeap.push(event);
    event->increaseReference();
    maxQsize = std::max(maxQsize, binomialHeap.size());  
}

void
BinomialHeapEventQueue::enqueue(muse::Agent* agent, muse::EventContainer& events) {
    UNUSED_PARAM(agent);
    ASSERT(agent != NULL);
    for(EventContainer::iterator curr = events.begin(); (curr != events.end());
        curr++) {
        muse::Event* event = *curr;
        binomialHeap.push(event);
    }
    maxQsize = std::max(maxQsize, binomialHeap.size());    
    // Clear out events in the container as per API expectations
    events.clear();   
}

int
BinomialHeapEventQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                           const muse::Time sentTime) {
    
    UNUSED_PARAM(dest);
    ASSERT(dest != NULL);
    int  numRemoved = 0;
     for (auto iter = binomialHeap.begin();
             iter != binomialHeap.end(); iter++) {
        Event * const evt = *iter;
        ASSERT(evt != NULL);
        // the antiMessage's and if the event is from same sender
        if ((evt->getSenderAgentID() == sender) &&
                (evt->getSentTime() >= sentTime)) {
             // Identify handle for current object on heap
            auto handle = BinomHeap::s_handle_from_iterator(iter);
            // Remove the event from the binomial heap
            binomialHeap.erase(handle);
            // This event needs to be canceled.
            evt->decreaseReference();

            numRemoved++;
        }
    }
    return numRemoved;
}

void
BinomialHeapEventQueue::prettyPrint(std::ostream& os) const {
    os << "BinomialHeapEventQueue [size=" << binomialHeap.size() << "]:\n";
    for (auto it = binomialHeap.ordered_begin(); it != binomialHeap.ordered_end(); ++it) {
        os << " " << (*it) << std::endl;
    }
    os << std::endl; 
}

void
BinomialHeapEventQueue::reportStats(std::ostream& os) {
    os << "BinomialHeapEventQueue:\n"
       << "\tMax queue size: " << maxQsize << std::endl;
}

END_NAMESPACE(muse)

#endif
