
#ifndef MedicState_CPP
#define MedicState_CPP

#include "MedicState.h"

MedicState::MedicState(){
    //insert ctor code here
}//end ctor

State*
MedicState::getClone(){
    //make sure you clone the state object correctly
    //check out muse examples for some hints
    //for primitive types shallow copy using default copy ctor should work, however for pointers or
    //class you need to do a deep copy.
    //MedicState *clone_state = new MedicState(this);
    //return clone_state;
}//end getClone

MedicState::~MedicState(){
    //insert dtor code here
}//end dtor

#endif /* MedicState_CPP */
