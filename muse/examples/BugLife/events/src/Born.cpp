#ifndef _BORN_CPP
#define _BORN_CPP

#include "Born.h"

Born::Born(AgentID receiverID, Time receiveTime,BugEventType e_type) :
BugEvent(receiverID, receiveTime, e_type ) {}

#endif
