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
#include "kernel/include/EventRecycler.h"

using namespace muse;

Event::Event(const AgentID  receiverID, const Time  receiveTime):
    receiverAgentID(receiverID), receiveTime(receiveTime),
    senderAgentID(-1), sentTime(TIME_INFINITY),  antiMessage(false),
    referenceCount(1), color('*'), inputRefCount(0) {
    // Nothing else to be done in the constructor.
}

char*
Event::allocate(const int size) {
    return muse::EventRecycler::allocate(size);
}

void
Event::deallocate(char* buffer, const int size) {
    return muse::EventRecycler::deallocate(buffer, size);
}

std::ostream&
operator<<(std::ostream& os, const muse::Event& event) {
    os << "Event[Sender=" << event.getSenderAgentID()   << ","
       << "receiver="     << event.getReceiverAgentID() << ","
       << "sentTime="     << event.getSentTime()        << ","
       << "recvTime="     << event.getReceiveTime()     << ","
       << "Anti-Message=" << event.isAntiMessage()      << ","
       << "Ref. count="   << event.getReferenceCount()  << ","
       << "color="        << (int) event.color          << ","
       << "ID="           << &event                     << "]";
    
    return os;
}

#endif
