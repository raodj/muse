/* 
  File:   Dead.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This event is used to notify the receiving agent that the sender
  bug has died.
 */

#ifndef _DEAD_H
#define	_DEAD_H
 
#include "BugEvent.h"
using namespace muse; 

class Dead : public BugEvent {
public:
    static Dead* create(const muse::AgentID id, const muse::Time t, BugEventType ty) {
        Dead* event = reinterpret_cast<Dead*>(new char[sizeof(Dead)]);
        new (event) BugEvent(id, t, ty);
        return event;
    }
    inline int getEventSize() {return sizeof(Dead); }
protected:
    Dead(AgentID receiverID,Time receiveTime, BugEventType e_type);
};

#endif	/* _DEAD_H */
