#ifndef VICTIM_STATE_CPP
#define VICTIM_STATE_CPP

#include "VictimState.h"

VictimState::VictimState() { 
   found = false; 
}

State* VictimState::getClone() {
   VictimState *clone_state = new VictimState();
   clone_state->timestamp   = getTimeStamp();
   clone_state->found       = found;
   return clone_state;
}

#endif
