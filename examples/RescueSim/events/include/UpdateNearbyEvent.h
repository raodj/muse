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
   inline void setNearbyVols(AgentID n[], int c) { 
      for(int i = 0; i < c; i++) nearbyVols[i] = n[i]; 
      nearbyVolCount = c; 
      messageFin = false;
   }
   inline void setNearbyVics(coord n[], int c) { 
      for(int i = 0; i < c; i++) nearbyVics[i] = n[i]; 
      nearbyVicCount = c; 
      messageFin = false;
   }
   inline AgentID* getNearbyVols() { return nearbyVols; }
   inline coord* getNearbyVics(){ return nearbyVics; }
   inline int getNearbyVolCount() { return nearbyVolCount; }
   inline int getNearbyVicCount() { return nearbyVicCount; }
   inline void setMessageFinal() { messageFin = true; }
   inline bool getMessageFinal() { return messageFin; }
protected:
   UpdateNearbyEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type);
   AgentID nearbyVols[MAX_EVENT_ARRAY_SIZE];
   int nearbyVolCount;
   coord nearbyVics[MAX_EVENT_ARRAY_SIZE];
   int nearbyVicCount;
   bool messageFin;
};

#endif /* UpdateNearbyEvent_H */