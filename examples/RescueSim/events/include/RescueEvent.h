#ifndef RESCUE_EVENT_H
#define RESCUE_EVENT_H

#include "Event.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class RescueEvent : public Event {
public:          
   static RescueEvent* create(const muse::AgentID receiverID, 
                              const muse::Time recvTime, VolunteerEventType type) { 
      RescueEvent* event = reinterpret_cast<RescueEvent*>(new char[sizeof(RescueEvent)]);          
      new (event) RescueEvent(receiverID, recvTime, type);
      return event;                                                   
   }
   inline int getEventSize() const { return sizeof(RescueEvent); }
   inline VolunteerEventType getEventType() const { return e_type; }
protected:
    VolunteerEventType e_type;
    RescueEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type);
};

#endif /* RESCUE_EVENT_H */