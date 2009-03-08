#ifndef _BUGEVENT_CPP
#define _BUGEVENT_CPP

#include "BugEvent.h"

BugEvent::BugEvent(AgentID receiverID, Time receiveTime, int e_type) :
Event(receiverID, receiveTime ), event_type(e_type) {}

#endif
