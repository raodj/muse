#ifndef _DEAD_CPP
#define _DEAD_CPP

#include "Dead.h"

Dead::Dead(AgentID receiverID, Time receiveTime,BugEventType e_type) :
BugEvent(receiverID, receiveTime, e_type ) {}

#endif
