#ifndef _CLOCKSTATE_CPP
#define _CLOCKSTATE_CPP

#include "BugState.h"

BugState::BugState() : size(1) {}

State*
BugState::getClone(){
    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    BugState* clone = new BugState();
    clone->setSize(size);
    clone->setLocation(getLocation());
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
