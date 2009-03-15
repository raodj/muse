#ifndef _SCOUT_CPP
#define _SCOUT_CPP

#include "Scout.h"

Scout::Scout(AgentID receiverID, Time receiveTime,BugEventType e_type) :
BugEvent(receiverID, receiveTime, e_type ), foodCount(0) {}

#endif
