#ifndef RescueEvent_H
#define RescueEvent_H

#include "Event.h"
#include "VolunteerDataTypes.h"

using namespace muse;

class RescueEvent : public Event {
public:
   RescueEvent(AgentID receiver_id, Time receive_time, VolunteerEventType type);
   inline int getEventSize() {return sizeof(RescueEvent);}
   inline VolunteerEventType getEventType() { return e_type; }
protected:
   VolunteerEventType e_type;
};

#endif /* RescueEvent_H */
