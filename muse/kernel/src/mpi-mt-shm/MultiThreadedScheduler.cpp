#ifndef MULTI_THREADED_SCHEDULER_CPP
#define MULTI_THREADED_SCHEDULER_CPP

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include "mpi-mt-shm/MultiThreadedScheduler.h"
#include <thread>

BEGIN_NAMESPACE(muse)

MultiThreadedScheduler::MultiThreadedScheduler() {
	std::cout << "Made an MT Scheduler" << std::endl;
}

MultiThreadedScheduler::~MultiThreadedScheduler() {}

bool
MultiThreadedScheduler::processNextAgentEvents(Time& simLGVT) {
    
    // === Dequeue next event set in thread safe way ===
    
    // Have the next agent (with lowest receive timestamp events) to
    // process its batch of events.
    // Do this in one method call so as to avoid race conditions
    agentPQ->dequeueNextAgentEvents(agentEvents);
    
    // If the event queue is empty, do no further operations.
    if (agentEvents.empty()) {
        // This thread has no more events to process, LGVT = infinity right now
        simLGVT = TIME_INFINITY;
        return false;
    }
    // Get the first of next batch of events to be scheduled.
    const muse::Event* const front = agentEvents.front();
    ASSERT(front != NULL);
    ASSERT(EventRecycler::getInputRefCount(front) < 2);
    
    // simLGVT is the time of the top agent
    simLGVT = agentEvents.front()->getReceiveTime()
    
    // === Process similar to Scheduler::processNextAgentEvents()
    
    DEBUG(std::cout << "Scheduler is processing event: " << *front
                    << std::endl);
    
    // Figure out the agent to receive this event.
    Agent* const agent = agentMap[front->getReceiverAgentID()];
    ASSERT(agent != NULL);
    
    // Since we're running concurrently, check for rollbacks even when processing
    if(checkAndHandleRollback(front, agent)) {
        agentPQ->enqueue(agent, agentEvents);
        agentEvents.clear();
        agent->agentLock.unlock();
        return true; // pretend like we processed events but skip this time step
    }
    
    // Check if the next lowest time-stamp event falls within time
    // window with respect to GVT.  If not, do not process events.
    if ((timeWindow > muse::Time(0)) && !withinTimeWindow(agent, front)) {
        // (deperomm) Doesn't this result in lost events? sholdn't agentEvents be rescheduled and the container cleared?
        agent->agentLock.unlock();
        return false;  // No events to schedule.
    }
    
    // Let the agent process its events and then release its lock
    agent->processNextEvents(agentEvents);
    agent->agentLock.unlock();
    agentEvents.clear();
    
    // Processed some events
    return true;
}

bool
MultiThreadedScheduler::scheduleEvent(Event* e) {
    // Make sure the recevier agent has an entry
    const AgentID agent_id = e->getReceiverAgentID();
    AgentIDAgentPointerMap::iterator entry = agentMap.find(agent_id);
    Agent* const agent = (entry != agentMap.end()) ? entry->second : NULL;
    
    if (agent == NULL) {
        std::cerr << "Trying to schedule (" << *e <<") to unknown agent\n";
        std::cerr << "Available agents are: \n";
        AgentIDAgentPointerMap::iterator it = agentMap.begin();
        for (; it!=agentMap.end(); it++) {
            std::cerr << *(it->second) << std::endl;
        }
        std::cerr << "Trying to schedule to local agent "
                  << "that doesn't exist" << std::endl;
        abort();
    }

    // Process rollbacks (only if necessary) 
    // optimistically schedule, handle rollbacks later *************************
//    checkAndHandleRollback(e, agent);
    
    // If the event is an anti-message then all pending future events
    // from the sender should also be deleted.
    if (e->isAntiMessage()) {
        // Clean-up any pending future events (from the sender of the
        // anti-message) in the scheduler's queue.
        handleFutureAntiMessage(e, agent);
        return false;  // Event not enqueued.
    }
    
    ASSERT(e->isAntiMessage() == false);
    // Actually add the event (enqueue indirectly increments reference count)
    agentPQ->enqueue(agent, e);
    // Event has been enqueued.
    return true;
}

END_NAMESPACE(muse)

#endif