#ifndef _CLOCKSTATE_CPP
#define	_CLOCKSTATE_CPP

#include "BugState.h"

BugState::BugState(int size, int x_pos, int y_pos) : size(0), x(x_pos), y(y_pos){}

State*
BugState::getClone(){
    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    ClockState * clone = new ClockState();
    clone->size = this->size;
	clone->x    = x;
	clone->y    = y;
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
