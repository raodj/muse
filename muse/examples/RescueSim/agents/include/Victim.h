
#ifndef Victim_H
#define Victim_H

#include "Agent.h"
#include "State.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class Victim : public Agent {
public:
   Victim(AgentID, State *, int c, int r);
   void initialize() throw (std::exception);
   void executeTask(const EventContainer* events);
   void finalize();
   int cols;
   int rows;
   coord my_location;
};

#endif /* Victim_H */
