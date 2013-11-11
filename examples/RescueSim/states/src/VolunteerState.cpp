#ifndef VolunteerState_CPP
#define VolunteerState_CPP

#include "VolunteerState.h"
#include <algorithm>

VolunteerState::VolunteerState() : curLocation(-1, -1) { }

State* VolunteerState::getClone(){
   VolunteerState *clone_state = new VolunteerState();
   clone_state->timestamp = getTimeStamp();
   clone_state->setLocation(getLocation());
   for(int i = 0; i < 8; i++) clone_state->moveTracker[i] = moveTracker[i];
   clone_state->CCCLocation = CCCLocation;
   clone_state->nearbyVolunteers = nearbyVolunteers;
   clone_state->knownVictims = knownVictims;
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
