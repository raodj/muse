#ifndef RescueArea_H
#define RescueArea_H

#include "Agent.h"
#include "State.h"
using namespace muse;

class RescueArea : public Agent {
public:
   RescueArea(AgentID, State *);
   void initialize() throw (std::exception);
   void executeTask(const EventContainer* events);
   void finalize();
};

#endif /* RescueArea_H */
