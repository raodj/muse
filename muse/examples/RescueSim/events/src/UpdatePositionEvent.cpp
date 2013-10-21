
#ifndef UpdatePositionEvent_CPP
#define UpdatePositionEvent_CPP

#include "UpdatePositionEvent.h"

UpdatePositionEvent::UpdatePositionEvent(AgentID receiver_id, Time receive_time, coord loc, VolunteerEventType type) 
	: RescueEvent(receiver_id, receive_time, type), cur_location(loc) { }

#endif /* UpdatePositionEvent_CPP */
