#ifndef _MOVEOUT_CPP
#define _MOVEOUT_CPP

#include "MoveOut.h"

MoveOut::MoveOut(AgentID receiverID, Time  receiveTime) :
BugEvent(receiverID,receiveTime), canBugMoveOut(false) {}

#endif
