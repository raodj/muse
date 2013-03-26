
#ifndef Volunteer_H
#define Volunteer_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.
    
    File: Volunteer.h
    Author: your name

    ........give brief description of what this  agent does here.......
*/

#include "Agent.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;

class Volunteer : public Agent {

public:
    Volunteer(AgentID, State *);
    void initialize() throw (std::exception);
    void executeTask(const EventContainer* events);
    void finalize();
    ~Volunteer();
};

#endif /* Volunteer_H */
