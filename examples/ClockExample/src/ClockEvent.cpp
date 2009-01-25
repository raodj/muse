#ifndef _CLOCKEVENT_CPP
#define	_CLOCKEVENT_CPP

#include "ClockEvent.h"

ClockEvent::ClockEvent(const AgentID& senderID, const AgentID& receiverID, const Time& sentTime, const Time& receiveTime) :
Event(senderID ,receiverID, sentTime, receiveTime ) {}

#endif
