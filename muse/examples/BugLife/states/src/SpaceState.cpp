#ifndef _SPACESTATE_CPP
#define	_SPACESTATE_CPP

#include "SpaceState.h"

SpaceState::SpaceState(int food_count, int x_pos, int y_pos) :
food(food_count), x(x_pos), y(y_pos), bugID(-1u), predID(-1u) {}

State*
SpaceState::getClone(){

    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    SpaceState * clone = new SpaceState();
    clone->food   = this->food;
	clone->x      = this->x;
	clone->y      = this->y;
	clone->bugID  = this->bugID;
	clone->predID = this->predID;
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
