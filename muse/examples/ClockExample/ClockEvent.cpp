#ifndef _CLOCKEVENT_CPP
#define	_CLOCKEVENT_CPP

#include "ClockEvent.h"

ClockEvent::ClockEvent(AgentID receiverID, Time receiveTime) :
Event(receiverID, receiveTime ) {}

#endif
 
