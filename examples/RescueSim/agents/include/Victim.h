
#ifndef Victim_H
#define Victim_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.
    
    File: Victim.h
    Author: your name

    ........give brief description of what this  agent does here.......
*/

#include "Agent.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;

class Victim : public Agent {

public:
    Victim(AgentID, State *);
    void initialize() throw (std::exception);
    void executeTask(const EventContainer* events);
    void finalize();
    ~Victim();
};

#endif /* Victim_H */
