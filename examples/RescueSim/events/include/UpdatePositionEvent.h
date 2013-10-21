
#ifndef UpdatePositionEvent_H
#define UpdatePositionEvent_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: UpdatePositionEvent.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class UpdatePositionEvent: public RescueEvent {
public:
   UpdatePositionEvent(AgentID receiver_id, Time receive_time, coord loc, VolunteerEventType type);
   inline int getEventSize() {return sizeof(UpdatePositionEvent);}
   coord cur_location;
};

#endif /* UpdatePositionEvent_H */
