#ifndef VolunteerState_H
#define VolunteerState_H

#include "State.h"
#include "VolunteerDataTypes.h"
#include <vector>
using namespace muse;

class VolunteerState : public State {
public:
   VolunteerState();
   State * getClone();
   inline coord getLocation() const {return curLocation;}
   inline void setLocation(coord new_coords) {
      curLocation.first = new_coords.first;
      curLocation.second = new_coords.second;
   }
   inline void setAgentID(AgentID i) { id = i; }
   inline std::vector<coord> getKnownVictims() const {return knownVictims;}
   inline std::vector<AgentID> getNearbyVolunteers() const {return nearbyVolunteers;}
   inline int* getMoveTracker() {return moveTracker;}
   void updateNearbyVols(AgentID* nearbyVols, int count);
   void updateKnownVics(coord* nearbyVics, int count);
protected:
   AgentID id;
   coord curLocation;
   int moveTracker[8];
   coord CCCLocation;
   std::vector<AgentID> nearbyVolunteers;
   std::vector<coord> knownVictims;
};

#endif /* VolunteerState_H */
