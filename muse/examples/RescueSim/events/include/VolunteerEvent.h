
#ifndef VolunteerEvent_H
#define VolunteerEvent_H

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
#include <vector>
using namespace muse;

class VolunteerEvent: public RescueEvent {
public:
   VolunteerEvent(AgentID receiver_id, Time receive_time,VolunteerEventType e_type);
   inline int getEventSize() { return sizeof(VolunteerEvent); }
   inline std::vector<coord> getNearbyVics() { return foundVictims; }
   std::vector<coord> foundVictims;
};

#endif /* VolunteerEvent_H */
