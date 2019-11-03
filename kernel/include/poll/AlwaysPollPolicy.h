#ifndef ALWAYS_POLL_POLICY_H
#define ALWAYS_POLL_POLICY_H

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

/** Always Poll Policy
	
	This is the naive implementation of a polling policy.
	This policy will invoke polling at every iteration of the 
	main simulation loop.
*/ 
class AlwaysPollPolicy : public PollPolicy {

public:
    AlwaysPollPolicy();
	virtual ~AlwaysPollPolicy();
    
	/** The shouldPoll method
		
		This policy will always decide to poll.
	*/
	virtual bool shouldPoll() override;
    
	/** The updatePolicy method.
		
		Updating this policy has no effect, as it always decides to poll.
	
		\param numEvents Number of events returned from previous poll
	*/
	virtual void updatePolicy(int numEvents = 0) override;
	
	/** The forcePoll method.
		
		Forcing this policy has no effect, as it always decides to poll.
	*/
	virtual void forcePoll() override;
};

END_NAMESPACE(muse);

#endif /* ALWAYS_POLL_POLICY_H */
