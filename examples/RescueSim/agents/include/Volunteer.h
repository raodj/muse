
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
#include "VolunteerDataTypes.h"
using namespace muse;

class Volunteer : public Agent {
//friend std::ostream& ::operator<<(ostream&, const Volunteer&);
public:
   Volunteer(AgentID, State *, CoordAgentIDMap *, int c, int r , int x, int y);
   void initialize() throw (std::exception);
   void executeTask(const EventContainer* events);
   void finalize();
   CoordAgentIDMap coord_map;
   int cols;
   int rows;
   coord my_location;
};

#endif /* Volunteer_H */
