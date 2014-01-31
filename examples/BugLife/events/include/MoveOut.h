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
    static MoveOut* create(const muse::AgentID receiverID, const muse::Time receiveTime, BugEventType e_type) {
        MoveOut* event = reinterpret_cast<MoveOut*>(new char[sizeof(MoveOut)]);
        new (event) MoveOut(receiverID, receiveTime, e_type);
        return event;
    }
    inline int getEventSize() { return sizeof(MoveOut); }
 protected:
    MoveOut(AgentID receiverID, Time receiveTime, BugEventType e_type);
};

#endif	/* _MOVEOUT_H */
