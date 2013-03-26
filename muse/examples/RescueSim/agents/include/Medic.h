
#ifndef Medic_H
#define Medic_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.
    
    File: Medic.h
    Author: your name

    ........give brief description of what this  agent does here.......
*/

#include "Agent.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;

class Medic : public Agent {

public:
    Medic(AgentID, State *);
    void initialize() throw (std::exception);
    void executeTask(const EventContainer* events);
    void finalize();
    ~Medic();
};

#endif /* Medic_H */
