#ifndef MUSE_EVENT_QUEUE_CPP
#define MUSE_EVENT_QUEUE_CPP

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "EventQueue.h"

// The static flag to indicate if events are directly shared between
// threads on a process.
bool muse::EventQueue::usingSharedEvents = false;

void
muse::EventQueue::setUsingSharedEvents(bool shareEvents) {
    usingSharedEvents = shareEvents;
}

#endif
