
#ifndef VolunteerEvent_H
#define VolunteerEvent_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: VolunteerEvent.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

#include "Event.h"
#include "VolunteerDataTypes.h"

using namespace muse;

class VolunteerEvent: public Event {
public:
    VolunteerEvent(AgentID receiver_id, Time receive_time,VolunteerEventType e_type);

    /** The getEventSize method.
        This is needed by muse kernel, do not erase.
        You can however do custom event size calculations.
    */
    inline int getEventSize() {return sizeof(VolunteerEvent);}
    inline VolunteerEventType getEventType()const{return event_type;}

protected:
   VolunteerEventType event_type;
   
};

#endif /* VolunteerEvent_H */
