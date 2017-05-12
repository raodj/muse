#ifndef _PCS_STATE_CPP
#define _PCS_STATE_CPP

#include "PCS_State.h"

PCS_State::PCS_State() : index(0) {}

muse::State*
PCS_State::getClone(){
    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primitive data types.
    PCS_State* clone = new PCS_State();
    clone->setIndex(this->getIndex());
    clone->timestamp = this->timestamp;
    
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif