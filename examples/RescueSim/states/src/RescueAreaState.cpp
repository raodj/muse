
#ifndef RescueAreaState_CPP
#define RescueAreaState_CPP

#include "RescueAreaState.h"

RescueAreaState::RescueAreaState(){
    //insert ctor code here
}//end ctor

State*
RescueAreaState::getClone(){
    //make sure you clone the state object correctly
    //check out muse examples for some hints
    //for primitive types shallow copy using default copy ctor should work, however for pointers or
    //class you need to do a deep copy.
    //RescueAreaState *clone_state = new RescueAreaState(this);
    //return clone_state;
}//end getClone

RescueAreaState::~RescueAreaState(){
    //insert dtor code here
}//end dtor

#endif /* RescueAreaState_CPP */
