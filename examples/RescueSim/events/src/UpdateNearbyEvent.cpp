
#ifndef UpdateNearbyEvent_CPP
#define UpdateNearbyEvent_CPP

#include "UpdateNearbyEvent.h"

UpdateNearbyEvent::UpdateNearbyEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type)
    : RescueEvent(receiver_id, receive_time, type){
   nearbyVolCount = 0;
   nearbyVicCount = 0;
   messageFin = false;
}

#endif /* UpdateNearbyEvent_CPP */
