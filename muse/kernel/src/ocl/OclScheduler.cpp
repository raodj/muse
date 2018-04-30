#ifndef MUSE_OCLSCHEDULER_CPP
#define MUSE_OCLSCHEDULER_CPP
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

#include "ocl/OclScheduler.h"

BEGIN_NAMESPACE(muse);

OclScheduler::OclScheduler() : Scheduler() {
    agentPQ = new AgentPQ();
}

AgentID
OclScheduler::processNextAgentEvents() {
    // If the event queue is empty, do no further operations.
    if (agentPQ->empty()) {
        return InvalidAgentID;
    }
    // Get the first of next batch of events to be scheduled.
    const muse::Event* const front = agentPQ->front();
    ASSERT(front != NULL);
    DEBUG(std::cout << "Scheduler is processing event: " << *front
                    << std::endl);
    // Figure out the agent to receive this event.
    oclAgent* const agent = reinterpret_cast<oclAgent*>
        (agentMap[front->getReceiverAgentID()]);
    ASSERT(agent != NULL);
    // Check if the next lowest time-stamp event falls within time
    // window with respect to GVT.  If not, do not process events.
    if ((timeWindow > muse::Time(0)) && !withinTimeWindow(agent, front)) {
        return InvalidAgentID;  // No events to schedule.
    }
    // Have the next agent (with lowest receive timestamp events) to
    // process its batch of events.
    agentPQ->dequeueNextAgentEvents(agentEvents);
    // create boolean to be passed as parameter and checked later
    bool runOCL = false;
    agent->processNextEvents(agentEvents, runOCL);

    agentEvents.clear();

    // check if should run equation for this agent
    // if so, set passed in agent to current agent
    // for it to be added to vector of agents waiting to be run
    if (runOCL) {
        return agent->getAgentID();
    } else {
        return InvalidOCLAgentID;
    }
}

END_NAMESPACE(muse);
#endif
