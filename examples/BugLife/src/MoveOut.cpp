#ifndef _MOVEOUT_CPP
#define _MOVEOUT_CPP

#include "MoveOut.h"

MoveOut::MoveOut(AgentID receiverID, Time  receiveTime, BugEventType e_type) :
BugEvent(receiverID,receiveTime, e_type){}

#endif
