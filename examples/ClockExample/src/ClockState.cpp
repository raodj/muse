#ifndef _CLOCKSTATE_CPP
#define	_CLOCKSTATE_CPP

#include "ClockState.h"

ClockState::ClockState() : hour(0) /*Note, we set default hour to Zero*/{}

State*
ClockState::getClone(){

    //keep in mind this shallow copy works because there are no complex
    //pointers that i need to take care of. Just primative datatypes.
    ClockState * clone = new ClockState();
    clone->hour = this->hour;
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}

#endif
