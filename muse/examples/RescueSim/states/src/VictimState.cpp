
#ifndef VictimState_CPP
#define VictimState_CPP

#include "VictimState.h"

VictimState::VictimState(){ found = false; }

State* VictimState::getClone() {
   VictimState *clone_state = new VictimState();
   return clone_state;
}

#endif /* VictimState_CPP */
