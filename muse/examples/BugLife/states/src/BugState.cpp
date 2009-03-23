#ifndef _CLOCKSTATE_CPP
#define _CLOCKSTATE_CPP

#include "BugState.h"

BugState::BugState() : size(1) , location(-1,-1),scoutReturned(0), bestScoutSpace(-1u, 0) {}

State*
BugState::getClone(){
    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    BugState* clone = new BugState();
    clone->setSize(size);
    clone->setLocation(getLocation());
    clone->setIsAlive(isAlive());
    clone->setScoutReturned(getScoutReturned());
    clone->setScoutSpace(getScoutSpace());
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
