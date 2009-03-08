/* 
  File:   BugEvent.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This is the base class from which all events in the bug simulation
  must derive from.
 */

#ifndef _BUGEVENT_H
#define	_BUGEVENT_H
 
#include "Event.h"
using namespace muse; 

class BugEvent : public Event {
public:
    BugEvent(AgentID receiverID,Time receiveTime,int e_type);
    inline int getEventSize() { return sizeof(BugEvent); }

    inline int  getEventType()const {return event_type;}
  
 protected:
    int event_type;
};

#endif	/* _BUGEVENT_H */

