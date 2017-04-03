#ifndef _EpidemicEvent_CPP
#define _EpidemicEvent_CPP

#include "EpidemicEvent.h"

EpidemicEvent::EpidemicEvent(muse::AgentID receiver_id, muse::Time receive_time,
        unsigned int arrvTime, Person indiv, EventType event_type_):
        Event(receiver_id, receive_time), locArrivalTimeStamp(arrvTime),
        person(indiv), event_type(event_type_) {        
}

#endif 