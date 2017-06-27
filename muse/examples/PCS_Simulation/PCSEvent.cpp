
#ifndef PCSEvent_CPP
#define PCSEvent_CPP

#include "PCSEvent.h"

PCSEvent::PCSEvent(muse::AgentID receiver_id, muse::Time receive_time,
                   muse::Time moveTime, muse::Time callTime,
                   muse::Time completionTime, Method method):
        Event(receiver_id, receive_time),
        moveTimeStamp(moveTime), nextCallTimeStamp(callTime), 
        completionTimeStamp(completionTime), method(method) {        
}

#endif /* PCSEvent_CPP */
