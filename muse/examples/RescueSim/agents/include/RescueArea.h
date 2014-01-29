#ifndef RescueArea_H
#define RescueArea_H

#include "Agent.h"
#include "State.h"
#include "VolunteerDataTypes.h"

using namespace muse;

class RescueArea : public Agent {
public:
   RescueArea(AgentID, State *);
   void initialize() throw (std::exception);
   void executeTask(const EventContainer*);
   void finalize();

private:
    // Helper method to dispatch nearby event
    void scheduleNearbyEvent(const AgentID,
                             const std::vector<AgentID>&,
                             const std::vector<coord>&,
                             const bool);
};

#endif /* RescueArea_H */
