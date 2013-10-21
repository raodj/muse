#ifndef UpdateNearbyEvent_H
#define UpdateNearbyEvent_H

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
#include <vector>
using namespace muse;

class UpdateNearbyEvent: public RescueEvent {
public:
   UpdateNearbyEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type);
   inline int getEventSize() {return sizeof(UpdateNearbyEvent);}
   inline void setNearbyVols(std::vector<AgentID> n) { nearbyVols = n; }
   inline void setNearbyVics(std::vector<coord> n) { nearbyVics = n; }
   inline std::vector<AgentID> getNearbyVols() { return nearbyVols; }
   inline std::vector<coord> getNearbyVics() { return nearbyVics; }
protected:
   std::vector<AgentID> nearbyVols;
   std::vector<coord> nearbyVics;
};

#endif /* UpdateNearbyEvent_H */
