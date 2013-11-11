#ifndef UpdateNearbyEvent_H
#define UpdateNearbyEvent_H

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
#include <vector>
using namespace muse;

class UpdateNearbyEvent: public RescueEvent {
public:
   static UpdateNearbyEvent* create(const muse::AgentID receiverID,
                                    const muse::Time recvTime, VolunteerEventType type) {
      UpdateNearbyEvent* event = reinterpret_cast<UpdateNearbyEvent*>(new char[sizeof(UpdateNearbyEvent)]);
      new (event) UpdateNearbyEvent(receiverID, recvTime, type);
      return event;
   }
   inline int getEventSize() {return sizeof(UpdateNearbyEvent);}
   inline void setNearbyVols(std::vector<AgentID> n) { nearbyVols = n; }
   inline void setNearbyVics(std::vector<coord> n) { nearbyVics = n; }
   inline std::vector<AgentID> getNearbyVols() { return nearbyVols; }
   inline std::vector<coord> getNearbyVics() { return nearbyVics; }
protected:
   UpdateNearbyEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type);
   std::vector<AgentID> nearbyVols;
   std::vector<coord> nearbyVics;
};

#endif /* UpdateNearbyEvent_H */
