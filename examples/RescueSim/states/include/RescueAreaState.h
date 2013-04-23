
#ifndef RescueAreaState_H
#define RescueAreaState_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: RescueAreaState.h
    Author: your name

    ........give brief description of what this  state contains here.......
*/

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
private:
   bool withinRange(coord a1, coord a2);
   int colID;
   std::vector<coord> victimLocations;
   map<muse::AgentID, coord> volunteerLocations;
};

#endif /* RescueAreaState_H */
