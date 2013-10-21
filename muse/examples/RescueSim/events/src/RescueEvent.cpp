
#ifndef RescueEvent_CPP
#define RescueEvent_CPP

#include "RescueEvent.h"

RescueEvent::RescueEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type) : Event(receiver_id, receive_time){
    e_type = type;
}

#endif /* RescueEvent_CPP */
