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
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include "Event.h"
#include "Simulation.h"

using namespace muse;

Event::Event(const AgentID  receiverID, const Time  receiveTime):
    senderAgentID(-1u), receiverAgentID(receiverID), sentTime(TIME_INFINITY), 
    receiveTime(receiveTime), antiMessage(false), referenceCount(1),
    color('*') {
    // Nothing else to be done in the constructor.
}

Event::~Event() {}

void
Event::decreaseReference(){
    ASSERT(getReferenceCount() >= 0);
    // Decrement the reference count.
    referenceCount--;
    if (referenceCount == 0) {
        char* buffer = reinterpret_cast<char*>(this);
        delete []  buffer;
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
       << "color="        << '0' + event.getColor()     << ","
       << "ID="           << &event                     << "]";
    
    return os;
}

#endif
