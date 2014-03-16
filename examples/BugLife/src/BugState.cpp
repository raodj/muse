#ifndef BUG_STATE_CPP
#define BUG_STATE_CPP

#include "BugState.h"
#include "MTRandom.h"

BugState::BugState() : location(-1, -1), scoutReturned(0),
		       bestScoutSpace(-1u, 0) {
    // Initialize size of the bug to a given size.
    size = int(MTRandom::RandDouble() * MAX_BUG_SIZE);
}

State*
BugState::getClone() {
    // Keep in mind this shallow copy works because there are no
    // complex pointers that i need to take care of.  Caller handles
    // deleting the memory when done with State pointer
    BugState* clone = new BugState();
    clone->setSize(size);
    clone->setLocation(getLocation());
    clone->setIsAlive(isAlive());
    clone->setScoutReturned(getScoutReturned());
    clone->setScoutSpace(getScoutSpace());
    clone->timestamp = this->timestamp;
    return clone;
}

#endif
