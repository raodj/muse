#ifndef _MUSE_STATE_CPP_
#define _MUSE_STATE_CPP_

#include "State.h"

using namespace muse;

State::State() : timestamp(0) {}

State* State::getClone(){
    //at a min we want to create a new state and set the timestamp
    State *state = new State();
    state->timestamp = getTimeStamp();
    return state;
}

State::~State(){}

#endif
