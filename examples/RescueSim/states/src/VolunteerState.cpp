
#ifndef VolunteerState_CPP
#define VolunteerState_CPP

#include "VolunteerState.h"

VolunteerState::VolunteerState() : curLocation(-1, -1) { }

State* VolunteerState::getClone(){
   VolunteerState *clone_state = new VolunteerState();
   clone_state->setLocation(getLocation());
   return clone_state;
}

#endif /* VolunteerState_CPP */
