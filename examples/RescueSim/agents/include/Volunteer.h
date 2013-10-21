#ifndef Volunteer_H
#define Volunteer_H

#include "Agent.h"
#include "State.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class Volunteer : public Agent {
public:
   Volunteer(AgentID, State *, CoordAgentIDMap *, int c, int r , int x, int y);
   void initialize() throw (std::exception);
   void executeTask(const EventContainer* events);
   void calculateMove();
   void finalize();
   CoordAgentIDMap coord_map;
   int cols;
   int rows;
   coord my_location;
};

#endif /* Volunteer_H */
