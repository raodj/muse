/* 
 * File:   ClockEvent.h
 * Author: gebremr
 *
 * Created on December 10, 2008, 11:30 PM
 */

#ifndef _CLOCKEVENT_H
#define	_CLOCKEVENT_H
 
#include "Event.h"
using namespace muse; 

class ClockEvent : public Event {
public:
    ClockEvent( AgentID receiverID, Time receiveTime);
    
};

#endif	/* _CLOCKEVENT_H */

