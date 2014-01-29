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

void VolunteerState::updateNearbyVols(AgentID* nearbyVols, int count) {
   nearbyVolunteers.clear();
   for(int i = 0; i < count; i++)
      nearbyVolunteers.push_back(nearbyVols[i]);
}

void VolunteerState::updateKnownVics(coord* foundVics, int count) {
  for(int i = 0; i < count; i++) {
      if(std::find(knownVictims.begin(), knownVictims.end(), foundVics[i]) == knownVictims.end()) {
//         std::cout << "Volunteer " << id << " found Victim at (" << foundVics[i].first << ", " 
//                   << foundVics[i].second << ").\n";
         knownVictims.push_back(foundVics[i]);
      }
   }
}

#endif /* VolunteerState_CPP */
