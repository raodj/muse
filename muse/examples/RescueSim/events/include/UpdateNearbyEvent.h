
#ifndef UpdateNearbyEvent_H
#define UpdateNearbyEvent_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: UpdateNearbyEvent.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

#include "Event.h"
#include "VolunteerDataTypes.h"
#include <vector>
using namespace muse;

class UpdateNearbyEvent: public Event {
public:
   UpdateNearbyEvent(AgentID receiver_id, Time receive_time);
   inline int getEventSize() {return sizeof(UpdateNearbyEvent);}
   std::vector<AgentID> nearbyVols;
   std::vector<coord> nearbyVics;
};

#endif /* UpdateNearbyEvent_H */
