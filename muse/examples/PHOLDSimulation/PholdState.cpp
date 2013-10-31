#ifndef _PHOLDSTATE_CPP
#define _PHOLDSTATE_CPP

#include "PholdState.h"

PholdState::PholdState() : index(0) {}

State*
PholdState::getClone(){
    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    PholdState* clone = new PholdState();
    clone->setIndex(this->getIndex());
    clone->timestamp = this->timestamp;
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
