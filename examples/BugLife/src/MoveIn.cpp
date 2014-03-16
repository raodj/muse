#ifndef _MOVEIN_CPP
#define _MOVEIN_CPP

#include "MoveIn.h"

MoveIn::MoveIn(AgentID receiverID, Time  receiveTime, BugEventType e_type) :
BugEvent(receiverID,receiveTime, e_type), canBugMoveIn(false) {}

#endif
