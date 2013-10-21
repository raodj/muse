
#ifndef VolunteerEvent_CPP
#define VolunteerEvent_CPP

#include "VolunteerEvent.h"

VolunteerEvent::VolunteerEvent(AgentID receiver_id, Time receive_time, VolunteerEventType e_type) 
	: RescueEvent(receiver_id, receive_time, e_type){ }

#endif /* VolunteerEvent_CPP */
