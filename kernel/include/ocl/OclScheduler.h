#ifndef MUSE_OCLSCHEDULER_H
#define MUSE_OCLSCHEDULER_H

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
// Authors: Harrison Roth          rothhl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "Scheduler.h"
#include "OclAgent.h"
BEGIN_NAMESPACE(muse);

const AgentID InvalidOCLAgentID = -2;

class OclAgent;
class OclScheduler : public Scheduler {
    friend class OclAgent;
    friend class OclSimulation;
    public:
        OclScheduler();
    protected:
    /** The current 'top' Agent is instructed to process its Events
        
     * \param agentOCL: this agent should be passed in as null. 
     *  if the agent is not null upon return, then opencl kernel code is 
     *  expected to process that agent
     * 
        \return True if the chosen agent had events to process.
    */
    AgentID processNextAgentEvents();

};
END_NAMESPACE(muse);
#endif
