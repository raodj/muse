#ifndef AVG_BACKOFF_POLICY_CPP
#define AVG_BACKOFF_POLICY_CPP

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

#include "poll/AvgBackoffPolicy.h"

using namespace muse;

AvgBackoffPolicy::AvgBackoffPolicy() {
    maxMpiMsgCheckThresh = 1000;
}

AvgBackoffPolicy::~AvgBackoffPolicy() {
}

bool AvgBackoffPolicy::shouldPoll() {
    return (--mpiMsgCheckCounter == 0);
}

void AvgBackoffPolicy::updatePolicy(int numEvents) {
    if (numEvents == 0) {
        if ((mpiMsgCheckThresh == 1) &&
            (maxMpiMsgCheckThresh.getCount() > 100)) {
            // Once we have collected sufficient samples we can
            // setup our threshold directly to that value.
            mpiMsgCheckThresh = maxMpiMsgCheckThresh.getMean();
        } else {
            mpiMsgCheckThresh = std::min(128, mpiMsgCheckThresh + 1);
        }
    } else {
        // Track the maximum value for mpiMsgCheckThresh for use
        // later on.
        maxMpiMsgCheckThresh += mpiMsgCheckThresh;
        mpiMsgCheckThresh     = 1;  // Reset back to 1.
    }
    mpiMsgCheckCounter = mpiMsgCheckThresh;
}

void AvgBackoffPolicy::forcePoll() {
    mpiMsgCheckCounter = 1;
}

#endif  // AVG_BACKOFF_POLICY_CPP
