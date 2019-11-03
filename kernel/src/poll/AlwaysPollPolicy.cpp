#ifndef ALWAYS_POLL_POLICY_CPP
#define ALWAYS_POLL_POLICY_CPP

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

#include "poll/AlwaysPollPolicy.h"

using namespace muse;

AlwaysPollPolicy::AlwaysPollPolicy() {
}

AlwaysPollPolicy::~AlwaysPollPolicy() {
}

bool AlwaysPollPolicy::shouldPoll() {
    return true; // always poll
}

void AlwaysPollPolicy::updatePolicy(int numEvents) {
	// updating policy has no effect
	// void parameter to suppress warning
	(void) numEvents;
}

void AlwaysPollPolicy::forcePoll() {
	// forcing policy has no effect
}

#endif /* ALWAYS_POLL_POLICY_CPP */
