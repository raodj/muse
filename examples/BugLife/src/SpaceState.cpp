#ifndef SPACESTATE_CPP
#define	SPACESTATE_CPP

#include "SpaceState.h"

SpaceState::SpaceState() :
food(5), bugID(-1u), predatorID(-1u) {}

muse::State*
SpaceState::getClone(){
    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primitive datatypes.
    SpaceState * clone = new SpaceState();
    clone->setFood(getFood());
    clone->setBugID(getBugID());
    clone->setPredatorID(getPredatorID());
    clone->timestamp = this->timestamp;
    return clone;
    //Caller will handle deleting the memory when done with State object
}

#endif
