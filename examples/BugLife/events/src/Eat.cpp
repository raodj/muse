#ifndef _EAT_CPP
#define _EAT_CPP

#include "Eat.h"

Eat::Eat(AgentID receiverID, Time receiveTime, int e_type) :
BugEvent(receiverID, receiveTime, e_type) {}

#endif
