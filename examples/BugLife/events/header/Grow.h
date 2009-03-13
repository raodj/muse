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
    Grow(AgentID receiverID,Time receiveTime, BugEventType e_type);
    inline int getEventSize() {return sizeof(Grow); }
};

#endif	/* _GROW_H */

