#ifndef MUSE_OCLSCHEDULER_CPP
#define MUSE_OCLSCHEDULER_CPP
#include "oclScheduler.h"
BEGIN_NAMESPACE(muse);

bool
oclScheduler::processNextAgentEvents(OCLAgent** oclAgent) {
    // If the event queue is empty, do no further operations.
    if (agentPQ->empty()) {
        return false;
    }
    // Get the first of next batch of events to be scheduled.
    const muse::Event* const front = agentPQ->front();
    ASSERT(front != NULL);
    DEBUG(std::cout << "Scheduler is processing event: " << *front
                    << std::endl);
    // Figure out the agent to receive this event.
    OCLAgent* const agent = (OCLAgent*)agentMap[front->getReceiverAgentID()];
    ASSERT(agent != NULL);    
    // Check if the next lowest time-stamp event falls within time
    // window with respect to GVT.  If not, do not process events.
    if ((timeWindow > muse::Time(0)) && !withinTimeWindow(agent, front)) {
        return false;  // No events to schedule.
    }
    // Have the next agent (with lowest receive timestamp events) to
    // process its batch of events.
    agentPQ->dequeueNextAgentEvents(agentEvents);
    
    //create boolean to be passed as param and checked later
    bool runOCL = false;
    agent->processNextEvents(agentEvents, runOCL);
    
    //check if should run equation for this agent
    //if so, set passed in agent to current agent
    //for it to be added to vector of agents waiting to be run
    if(runOCL){
        *oclAgent = agent;
    }else{
        *oclAgent = NULL;
    }
    agentEvents.clear();
    // Processed some events
    return true;
}

END_NAMESPACE(muse);
#endif