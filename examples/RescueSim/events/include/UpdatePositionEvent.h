#ifndef UpdatePositionEvent_H
#define UpdatePositionEvent_H

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class UpdatePositionEvent: public RescueEvent {
public:
   UpdatePositionEvent(AgentID receiver_id, Time receive_time, coord loc, VolunteerEventType type);
   inline int getEventSize() {return sizeof(UpdatePositionEvent);}
   coord cur_location;
};

#endif /* UpdatePositionEvent_H */
