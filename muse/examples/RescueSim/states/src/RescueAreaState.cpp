
#ifndef RescueAreaState_CPP
#define RescueAreaState_CPP

#include "RescueAreaState.h"
#include "math.h"

RescueAreaState::RescueAreaState(int col) : colID(col){ }

State* RescueAreaState::getClone(){
   RescueAreaState *clone_state = new RescueAreaState(colID);
   return clone_state;
}

void RescueAreaState::addVictim(coord loc) {
   victimLocations.push_back(loc);
}

void RescueAreaState::updateVolunteerPosition(muse::AgentID id, coord loc) {
   if(loc.second/AREA_COL_WIDTH == colID) volunteerLocations[id] = loc;
   else volunteerLocations.erase(id);
}

std::vector<coord> RescueAreaState::getNearbyVictims(muse::AgentID id) {
   std::vector<coord> nearby;
   for(std::vector<coord>::iterator it = victimLocations.begin(); it != victimLocations.end(); it++)
      if(withinRange(volunteerLocations[id], *it))
         nearby.push_back(*it);
   return nearby;
}

std::vector<muse::AgentID> RescueAreaState::getNearbyVolunteers(muse::AgentID id) {
   std::vector<muse::AgentID> nearby;
   for(std::map<muse::AgentID, coord>::iterator it = volunteerLocations.begin(); it != volunteerLocations.end(); it++)
      if(it->first != id && withinRange(volunteerLocations[id], it->second))
         nearby.push_back(it->first);
   return nearby;
}

bool RescueAreaState::withinRange(coord a1, coord a2) {
   return (sqrt(pow((a2.first-a1.first), 2) + pow((a2.second-a1.second), 2)) <= VOL_SIGNAL_RANGE);
}

#endif /* RescueAreaState_CPP */
