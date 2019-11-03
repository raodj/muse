#ifndef RL_BACKOFF_POLICY_H
#define RL_BACKOFF_POLICY_H

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

#include "poll/PollPolicy.h"

BEGIN_NAMESPACE(muse);

class RLBackoffPolicy : public PollPolicy {
public:
    RLBackoffPolicy();
    virtual ~RLBackoffPolicy();
    virtual bool shouldPoll() override;
    virtual void updatePolicy(int numEvents = 0) override;
    virtual void forcePoll() override;
protected:
    
        
};

END_NAMESPACE(muse);

#endif /* RL_BACKOFF_POLICY_H */
