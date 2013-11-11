
#ifndef VolunteerEvent_H
#define VolunteerEvent_H

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
#include <vector>
using namespace muse;

class VolunteerEvent: public RescueEvent {
public:
   static VolunteerEvent* create(const muse::AgentID receiverID,
                                 const muse::Time recvTime, VolunteerEventType type) {
      VolunteerEvent* event = reinterpret_cast<VolunteerEvent*>(new char[sizeof(VolunteerEvent)]);
      new (event) VolunteerEvent(receiverID, recvTime, type);
      return event;
   }
   inline int getEventSize() { return sizeof(VolunteerEvent); }
   inline std::vector<coord> getNearbyVics() { return foundVictims; }
   std::vector<coord> foundVictims;
protected:
   VolunteerEvent(AgentID receiver_id, Time receive_time,VolunteerEventType e_type);
};

#endif /* VolunteerEvent_H */
