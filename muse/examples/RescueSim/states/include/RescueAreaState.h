#ifndef RescueAreaState_H
#define RescueAreaState_H

#include "State.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class RescueAreaState : public State {
public:
   RescueAreaState(int col);
   State * getClone();
   void addVictim(coord loc);
   void updateVolunteerPosition(muse::AgentID id, coord loc);
   std::vector<coord> getNearbyVictims(muse::AgentID id);
   std::vector<muse::AgentID> getNearbyVolunteers(muse::AgentID id);
   inline int getColID() { return colID; }
private:
   bool withinRange(coord a1, coord a2);
   int colID;
   std::vector<coord> victimLocations;
   map<muse::AgentID, coord> volunteerLocations;
};

#endif /* RescueAreaState_H */