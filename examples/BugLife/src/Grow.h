/* 
  File:   Grow.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This event is used to request a bug to grow. If a bug gets this event
  the bug will grow and it reaches its max size it will give birth and die.
 */


#ifndef _GROW_H
#define	_GROW_H
 
#include "BugEvent.h"
using namespace muse; 

class Grow : public BugEvent {
 public:
    static Grow* create(const muse::AgentID receiverID,
			const muse::Time receiveTime,
			BugEventType e_type = GROW) {
        Grow* event = reinterpret_cast<Grow*>(new char[sizeof(Grow)]);
        new (event) Grow(receiverID, receiveTime, e_type);
        return event;
    }
    inline int getEventSize() const { return sizeof(Grow); }
    inline void setSize(int growth_size) { size=growth_size; }
    int size;

 protected:
    Grow(AgentID receiverID,Time receiveTime, BugEventType e_type);
};

#endif	/* _GROW_H */

