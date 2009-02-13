#ifndef _MUSE_STATE_CPP_
#define _MUSE_STATE_CPP_

#include "State.h"

using namespace muse;

State::State() : timestamp(0) {}

State* State::getClone(){
    State *state = new State();
    state->timestamp = getTimeStamp();
    return state;
}


const Time &
State::getTimeStamp(){
    return timestamp ;
}
State::~State(){}

#endif
