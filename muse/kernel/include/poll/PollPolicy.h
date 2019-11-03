#ifndef POLL_POLICY_H
#define POLL_POLICY_H

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

#include "Utilities.h"

BEGIN_NAMESPACE(muse);

class PollPolicy {
public:
    PollPolicy();
    virtual ~PollPolicy();
    virtual bool shouldPoll() = 0;
    virtual void updatePolicy(int numEvents = 0) = 0;
    virtual void forcePoll() = 0;
};

END_NAMESPACE(muse);

#endif /* POLL_POLICY_H */
