#ifndef _MOVEIN_CPP
#define _MOVEIN_CPP

#include "MoveIn.h"

MoveIn::MoveIn(AgentID receiverID, Time  receiveTime) :
BugEvent(receiverID,receiveTime), canBugMoveIn(false) {}

#endif
