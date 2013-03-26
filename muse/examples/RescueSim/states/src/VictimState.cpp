
#ifndef VictimState_CPP
#define VictimState_CPP

#include "VictimState.h"

VictimState::VictimState(){
    //insert ctor code here
}//end ctor

State*
VictimState::getClone(){
    //make sure you clone the state object correctly
    //check out muse examples for some hints
    //for primitive types shallow copy using default copy ctor should work, however for pointers or
    //class you need to do a deep copy.
    //VictimState *clone_state = new VictimState(this);
    //return clone_state;
}//end getClone

VictimState::~VictimState(){
    //insert dtor code here
}//end dtor

#endif /* VictimState_CPP */
