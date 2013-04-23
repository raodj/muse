
#ifndef VolunteerState_H
#define VolunteerState_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: VolunteerState.h
    Author: your name

    ........give brief description of what this  state contains here.......
*/

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
   inline std::vector<coord> getKnownVictims() const {return knownVictims;}
   inline void addVictim(coord location) {knownVictims.push_back(location);}
protected:
   coord curLocation;
   coord CDCLocation;
   std::vector<coord> knownVictims;
};

#endif /* VolunteerState_H */
