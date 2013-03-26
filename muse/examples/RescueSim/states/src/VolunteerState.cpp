
#ifndef VolunteerState_CPP
#define VolunteerState_CPP

#include "VolunteerState.h"

VolunteerState::VolunteerState(){
    //insert ctor code here
}//end ctor

State*
VolunteerState::getClone(){
    //make sure you clone the state object correctly
    //check out muse examples for some hints
    //for primitive types shallow copy using default copy ctor should work, however for pointers or
    //class you need to do a deep copy.
    //VolunteerState *clone_state = new VolunteerState(this);
    //return clone_state;
}//end getClone

VolunteerState::~VolunteerState(){
    //insert dtor code here
}//end dtor

#endif /* VolunteerState_CPP */
