/* 
  File:   MoveOut.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This event is used to request a move out of a space. A bug must send this event
  and the space will  accept. Once the bug gets a MoveOut event back it will know that the space 
  has accepted the move out and will release its record of the space.
 */

#ifndef _MOVEOUT_H
#define	_MOVEOUT_H
 
#include "BugEvent.h"
using namespace muse; 

class MoveOut : public BugEvent {
public:
    MoveOut(AgentID receiverID, Time  receiveTime);
	bool canBugMoveOut; 
    inline int getEventSize() {return sizeof(MoveOut); }
};

#endif	/* _MOVEOUT_H */