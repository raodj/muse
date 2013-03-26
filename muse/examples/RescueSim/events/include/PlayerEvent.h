
#ifndef PlayerEvent_H
#define PlayerEvent_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: PlayerEvent.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

#include "Event.h"
#include "DataTypes.h"

using namespace muse;

class PlayerEvent : public Event {
public:
    PlayerEvent(AgentID receiver_id, Time receive_time);

    /** The getEventSize method.
        This is needed by muse kernel, do not erase.
        You can however do custom event size calculations.
    */
    inline int getEventSize() {return sizeof(PlayerEvent);}
    inline PlayerEventType getEventType()const{return event_type;}

    ~PlayerEvent();
protected:
   PlayerEventType event_type;
};

#endif /* PlayerEvent_H */
