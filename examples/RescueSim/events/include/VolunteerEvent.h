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
   inline int getEventSize() const { return sizeof(VolunteerEvent); }
   inline void setFoundVics(coord n[], int c) {
      for(int i = 0; i < c; i++) foundVictims[i] = n[i];
      foundVicCount = c;
   }
   inline coord* getNearbyVics() { return foundVictims; }
   inline int getNearbyVicCount() { return foundVicCount; }
protected:
   coord foundVictims[MAX_EVENT_ARRAY_SIZE];
   int foundVicCount;
   VolunteerEvent(AgentID receiver_id, Time receive_time,VolunteerEventType e_type);
};

#endif /* VolunteerEvent_H */
