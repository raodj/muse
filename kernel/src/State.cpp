#ifndef _MUSE_STATE_CPP_
#define _MUSE_STATE_CPP_

#include "State.h"

using namespace muse;

State::State(){}

State* State::getClone(){
    State *state = new State();
    return state;
}

State::~State(){}

#endif
