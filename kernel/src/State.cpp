#ifndef _MUSE_STATE_H_
#include "../../include/State.h"
#endif
using muse::State;

State::State(){}

State* State::getClone(){

	return this;
}

State::~State(){}