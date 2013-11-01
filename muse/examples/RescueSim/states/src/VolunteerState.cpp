#ifndef VolunteerState_CPP
#define VolunteerState_CPP

#include "VolunteerState.h"
#include <algorithm>

VolunteerState::VolunteerState() : curLocation(-1, -1) { }

State* VolunteerState::getClone(){
   VolunteerState *clone_state = new VolunteerState();
   clone_state->setLocation(getLocation());
   return clone_state;
}

void VolunteerState::updateNearbyVols(std::vector<AgentID> nearbyVols) {
   nearbyVolunteers.clear();
   nearbyVolunteers = nearbyVols;
}

void VolunteerState::updateKnownVics(std::vector<coord> foundVics) {
   for(std::vector<coord>::iterator it = foundVics.begin(); it != foundVics.end(); it++) {
      if(std::find(knownVictims.begin(), knownVictims.end(), (*it)) == knownVictims.end()) {
         std::cout << "Volunteer " << id << " found Victim at (" << (*it).first << ", " << (*it).second << ").\n";
         knownVictims.push_back(*it);
      }
   }
}

#endif /* VolunteerState_CPP */
