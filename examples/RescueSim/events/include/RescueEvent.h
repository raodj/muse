
#ifndef RescueEvent_H
#define RescueEvent_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: RescueEvent.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

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
