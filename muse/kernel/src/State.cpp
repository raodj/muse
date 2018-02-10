#ifndef MUSE_STATE_CPP
#define MUSE_STATE_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "State.h"
#include "kernel/include/StateRecycler.h"

using namespace muse;

State::State() : timestamp(0) {}

State::~State() {}

State*
State::getClone() {
    State *state = new State();
    state->timestamp = getTimeStamp();
    return state;
}

// Overloaded new operator for all states to streamline recycling of
// memory.
void*
State::operator new(size_t sz) {
    ASSERT(sz >= sizeof(muse::State));
    return StateRecycler::allocate(sz);
}

// Overloaded delete operator to enable recycling of states.
void
State::operator delete(void *state) {
    return StateRecycler::deallocate(static_cast<char*>(state));
}

#endif
