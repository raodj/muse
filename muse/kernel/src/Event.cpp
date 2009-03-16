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

Event::Event(const AgentID  receiverID, const Time  receiveTime):
    senderAgentID(-1u), sentTime(TIME_INFINITY),receiverAgentID(receiverID),receiveTime(receiveTime),
    antiMessage(false),referenceCount(0) {
    // Nothing else to be done in the constructor.
}

Event::~Event(){}

void
Event::decreaseReference(){
    ASSERT ( referenceCount >= 0 );
    // Declreate reference count.
    if (!referenceCount) {
        
        //std::cout << "Getting deleted: " << *this << std::endl;
        if (Simulation::getSimulator()->isAgentLocal(senderAgentID)) {
            // This event was allocated using new operator as a
            // standard event. Do delete it appropriately.
            delete this;
        } else{
            //cout << "Event was not local: calling delete[]\n\n";
            // This event was received over the wire. Therefore
            // this event must be deleted as an array of characters
            char* buffer = reinterpret_cast<char*>(this);
            delete []  buffer;
        }
        
    }else referenceCount--;
}//end decreaseReference

void
Event::increaseReference(){
    //std::cout << "Increasing ref" << std::endl;
    referenceCount++;
}//end increaseReference

void
Event::makeAntiMessage() {
    antiMessage=true;
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
       << "Ref. count="   << event.referenceCount       << ","
       << "color="        << event.getColor()           << "]";
    
    return os;
}


#endif

