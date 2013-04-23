
#ifndef UpdatePositionEvent_CPP
#define UpdatePositionEvent_CPP

#include "UpdatePositionEvent.h"

UpdatePositionEvent::UpdatePositionEvent(AgentID receiver_id, Time receive_time, coord loc, VolunteerEventType type) 
	: Event(receiver_id, receive_time), cur_location(loc), e_type(type) { }

#endif /* UpdatePositionEvent_CPP */
