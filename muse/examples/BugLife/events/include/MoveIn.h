/* 
  File:   MoveIn.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This event is used to request a move into a space. A bug must send this event
  and the space will either accept/decline based on some constraint. Once the 
  bug gets a MoveIn event from the same space it will know to move or not.
 */

#ifndef _MOVEIN_H
#define	_MOVEIN_H
 
#include "BugEvent.h"
using namespace muse; 

class MoveIn : public BugEvent {
 public:
    static MoveIn* create(const muse::AgentID receiverID, const muse::Time receiveTime, BugEventType e_type) {
        MoveIn* event = reinterpret_cast<MoveIn*>(new char[sizeof(MoveIn)]);
        new (event) MoveIn(receiverID, receiveTime, e_type);
        return event;
    }
    inline int getEventSize() { return sizeof(MoveIn); }
    bool canBugMoveIn;
 protected:
    MoveIn(AgentID receiverID, Time receiveTime, BugEventType e_type);
};

#endif	/* _MOVEIN_H */

