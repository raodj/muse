
#ifndef UpdateNearby_H
#define UpdateNearby_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: UpdateNearby.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

#include "Event.h"
#include "DataTypes.h"

using namespace muse;

class UpdateNearby : public Event {
public:
    UpdateNearby(AgentID receiver_id, Time receive_time);

    /** The getEventSize method.
        This is needed by muse kernel, do not erase.
        You can however do custom event size calculations.
    */
    inline int getEventSize() {return sizeof(UpdateNearby);}

    ~UpdateNearby();
};

#endif /* UpdateNearby_H */
