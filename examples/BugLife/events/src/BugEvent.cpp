#ifndef _BUGEVENT_CPP
#define	_BUGEVENT_CPP

#include "BugEvent.h"

BugEvent::BugEvent(AgentID receiverID, Time receiveTime) :
Event(receiverID, receiveTime ) {}

#endif
