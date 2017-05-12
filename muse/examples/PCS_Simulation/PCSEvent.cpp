
#ifndef PCSEvent_CPP
#define PCSEvent_CPP

#include "PCSEvent.h"

PCSEvent::PCSEvent(muse::AgentID receiver_id, muse::Time receive_time,
        unsigned int moveTime, unsigned int callTime,
        unsigned int completionTime, Method method_):
        Event(receiver_id, receive_time),
        moveTimeStamp(moveTime), nextCallTimeStamp(callTime), 
        completionTimeStamp(completionTime), method(method_) {        
}

#endif /* PCSEvent_CPP */
