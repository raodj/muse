#ifndef EXP_BACKOFF_POLICY_CPP
#define EXP_BACKOFF_POLICY_CPP

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

#include "poll/ExpBackoffPolicy.h"

using namespace muse;

ExpBackoffPolicy::ExpBackoffPolicy() {
}

ExpBackoffPolicy::~ExpBackoffPolicy() {
}

bool ExpBackoffPolicy::shouldPoll() {
    return (--mpiMsgCheckCounter == 0);
}

void ExpBackoffPolicy::updatePolicy(int numEvents) {
    if (numEvents == 0) {
        mpiMsgCheckThresh = std::min(10240, mpiMsgCheckThresh * 2);
    } else {
        mpiMsgCheckThresh = std::min(1, mpiMsgCheckThresh / 2);
    }
    mpiMsgCheckCounter = mpiMsgCheckThresh;
}

void ExpBackoffPolicy::forcePoll() {
    mpiMsgCheckCounter = 1;
}

#endif  /* EXP_BACKOFF_POLICY_CPP */
