#ifndef _SPACESTATE_CPP
#define	_SPACESTATE_CPP

#include "SpaceState.h"

SpaceState::SpaceState() :
food(5), bugID(-1u), predatorID(-1u) {}

State*
SpaceState::getClone(){

    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    SpaceState * clone = new SpaceState();
   	clone->setFood(getFood());
	clone->setBugID(getBugID());
	clone->setPredatorID(getPredatorID());
	
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
