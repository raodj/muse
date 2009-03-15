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
    Dead(AgentID receiverID,Time receiveTime, BugEventType e_type);
    inline int getEventSize() {return sizeof(Dead); }

};

#endif	/* _DEAD_H */
