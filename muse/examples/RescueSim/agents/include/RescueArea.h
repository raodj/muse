
#ifndef RescueArea_H
#define RescueArea_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.
    
    File: RescueArea.h
    Author: your name

    ........give brief description of what this  agent does here.......
*/

#include "Agent.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;

class RescueArea : public Agent {

public:
    RescueArea(AgentID, State *);
    void initialize() throw (std::exception);
    void executeTask(const EventContainer* events);
    void finalize();
    ~RescueArea();
};

#endif /* RescueArea_H */
