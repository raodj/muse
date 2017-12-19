#ifndef Volunteer_H
#define Volunteer_H

#include "Agent.h"
#include "State.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class Volunteer : public Agent {
public:
   Volunteer(AgentID, State *, int c, int r , int x, int y);
   void initialize() throw (std::exception) override;
   void executeTask(const EventContainer&) override;
   void calculateMove();
   void finalize() override;
protected:
   int cols;
   int rows;
   coord my_location;
private:
   void scheduleUpdatePosEvent(const bool);
};

#endif /* Volunteer_H */
