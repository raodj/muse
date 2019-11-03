#ifndef RL_BACKOFF_POLICY_CPP
#define RL_BACKOFF_POLICY_CPP

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
// Authors: Chris Benton           bentoncl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "poll/RLBackoffPolicy.h"

using namespace muse;

RLBackoffPolicy::RLBackoffPolicy() {
}

RLBackoffPolicy::~RLBackoffPolicy() {
}

bool RLBackoffPolicy::shouldPoll() {
    return true;
}

void RLBackoffPolicy::updatePolicy(int numEvents) {
	(void) numEvents; // suppress unused paramters warning
}

void RLBackoffPolicy::forcePoll() {
}

#endif /* RL_BACKOFF_POLICY_CPP */
