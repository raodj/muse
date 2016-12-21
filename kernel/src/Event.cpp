#ifndef MUSE_EVENT_CPP
#define	MUSE_EVENT_CPP

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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "Event.h"
#include "Simulation.h"

using namespace muse;

// The static map used for recycling events
std::unordered_map<int, std::stack<char*>> Event::EventRecycler;

Event::Event(const AgentID  receiverID, const Time  receiveTime):
    senderAgentID(-1u), receiverAgentID(receiverID), sentTime(TIME_INFINITY), 
    receiveTime(receiveTime), antiMessage(false), referenceCount(1),
    color('*') {
    // Nothing else to be done in the constructor.
}

Event::~Event() {
    // Nothing else to be done in the destructor.
}

void
Event::decreaseReference() {
    ASSERT(getReferenceCount() > 0);
    // Decrement the reference count.
    referenceCount--;
    if (referenceCount == 0) {
        // Manually call event destructor
        this->~Event();
        // Free-up or recycle the memory for this event.
#ifdef RECYCLE_EVENTS        
        deallocate(reinterpret_cast<char*>(this), this->getEventSize());
#else
        // Avoid 1 extra call to getEventSize() in this situation
        deallocate(reinterpret_cast<char*>(this), 0);
#endif
    }
}

void
Event::increaseReference(){
    referenceCount++;
    ASSERT(referenceCount < 4);
}

void
Event::makeAntiMessage() {
    antiMessage = true;
}

void
Event::setColor(const char col) {
    color = col;
}

ostream&
operator<<(ostream& os, const muse::Event& event) {
    os << "Event[Sender=" << event.getSenderAgentID()   << ","
       << "receiver="     << event.getReceiverAgentID() << ","
       << "sentTime="     << event.getSentTime()        << ","
       << "recvTime="     << event.getReceiveTime()     << ","
       << "Anti-Message=" << event.isAntiMessage()      << ","
       << "Ref. count="   << event.getReferenceCount()  << ","
       << "color="        << (int) event.getColor()     << ","
       << "ID="           << &event                     << "]";
    
    return os;
}

// ------------[ Methods associated with event recycling ]--------------

char*
Event::allocate(const int size) {
#ifdef RECYCLE_EVENTS
    std::unordered_map<int, std::stack<char*>>::iterator curr =
        EventRecycler.find(size);
    if (curr != EventRecycler.end() && !curr->second.empty()) {
        // Recycle existing buffer.
        char *buf = curr->second.top();
        curr->second.pop();
        return buf;
    }
#endif
    // No existing buffer of given size to recycle (or recycling is
    // disabled). So, create a new one.
    return new char[size];
}

void
Event::deallocate(char* buffer, const int size) {
#ifdef RECYCLE_EVENTS
    EventRecycler[size].push(buffer);
#else
    UNUSED_PARAM(size);
    delete [] buffer;
#endif
}

void
Event::deleteRecycledEvents() {
    std::unordered_map<int, std::stack<char*>>::iterator curr;
    for (curr = EventRecycler.begin(); (curr != EventRecycler.end()); curr++) {
        std::stack<char*>& stack = curr->second;
        while (!stack.empty()) {
            delete [] stack.top();
            stack.pop();
        }
    }
    EventRecycler.clear();
}

#endif
