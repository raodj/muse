#ifndef GVT_MESSAGE_CPP
#define	GVT_MESSAGE_CPP

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
// Authors:  Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "GVTMessage.h"
#include "Event.h"
#include "config.h"
#include "EventRecycler.h"

// Switch to muse name space to make life easier.
using namespace muse;

GVTMessage*
GVTMessage::create(const GVTMsgKind msgKind, const int numProcesses,
                   int destRank) {
    // A static variable to generate sequence numbers
    static unsigned int GlobalSequenceCounter = 0;
    
    // First compute the message size.
    const int vecSize = (msgKind != GVT_CTRL_MSG) ? 0 :
        (sizeof(unsigned int) * numProcesses);
    const int msgSize = sizeof(GVTMessage) + vecSize;
    // Allocate flat memory for the message.
    char* memory = Event::allocate(msgSize, -1);
    // Now use the flat memory to instantiate an object.
    GVTMessage* msg = new (memory) GVTMessage(msgKind, msgSize);
    // Setup the sequence number for this newly created message
    msg->receiverAgentID = destRank;
    msg->sequenceNumber  = GlobalSequenceCounter++;
    // Now we have a message object to return/work-with
    return msg;
}

GVTMessage*
GVTMessage::create(const GVTMessage* src, int destRank, int destThread) {
    UNUSED_PARAM(destThread);
    const int msgSize = src->getSize();
    // Allocate flat memory for the message with NUMA awareness.
#if USE_NUMA == 1
    char* memory = EventRecycler::allocateNuma(msgSize, 0);
#else
    char* memory = Event::allocate(msgSize, -1);
#endif
    // Now use the flat memory to instantiate an object.
    GVTMessage* msg = new (memory) GVTMessage(src->kind, msgSize);
    // Copy all the information
    std::memcpy(msg, src, msgSize);
    // Setup the receiver thread/process so that receiving process can
    // route GVT messages appropriately.
    msg->receiverAgentID = destRank;
    // Now we have a message object to return/work-with
    return msg;
}

void
GVTMessage::destroy(GVTMessage* msg) {
    // Let the Event's deallocation/recycling take care of this message
    muse::Event::deallocate(msg);
}

void
GVTMessage::setTmin(const Time& min) {
    tMin = min;
}

void
GVTMessage::setGVTEstimate(const Time& estimate) {
    gvtEstimate = estimate;
}

bool
GVTMessage::areCountersZero(const int numProcesses) const {
    ASSERT(kind == GVT_CTRL_MSG);
    for(int pid = 0; (pid < numProcesses); pid++) {
        if (count[pid]) {
            // There is at least one non-zero entry.
            return false;
        }
    }
    // All entries are zeros
    return true;
}

GVTMessage::GVTMessage(const GVTMsgKind msgKind, const int msgSize)
    : Event(-1, -1), kind(msgKind), size(msgSize) {
    // Initialize other members to invalid values.
    gvtEstimate = tMin = TIME_INFINITY;
    // Set variables in muse::Event base class to invalid values
    // senderAgentID = -1;
    // sentTime      = -1;
}

std::ostream&
operator<<(std::ostream& os, const muse::GVTMessage& gvtMsg) {
    os << "GVTMessage(kind=" << gvtMsg.kind << ", " << "size="
       << gvtMsg.size << ", seq#=" << gvtMsg.sequenceNumber
       << "): gvtEstimate=" << gvtMsg.gvtEstimate
       << ", tMin = "<< gvtMsg.tMin << ". Vector counters = { ";

    const int MaxCounters = (gvtMsg.size - sizeof(muse::GVTMessage)) /
        sizeof(int);
    for (int i = 0; (i < MaxCounters); i++) {
        os << gvtMsg.count[i] << " ";
    }
    os << "}";
    return os;
}

#endif
