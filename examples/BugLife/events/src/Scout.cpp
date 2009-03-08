#ifndef _SCOUT_CPP
#define _SCOUT_CPP

#include "Scout.h"

Scout::Scout(AgentID receiverID, Time receiveTime,int e_type) :
BugEvent(receiverID, receiveTime, e_type ) {}

#endif
