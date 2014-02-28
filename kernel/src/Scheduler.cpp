#ifndef MUSE_SCHEDULER_CPP
#define MUSE_SCHEDULER_CPP

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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include "Scheduler.h"
#include <cstdlib>
#include "Simulation.h"
#include "BinaryHeapWrapper.h"

using namespace muse;

Scheduler::Scheduler() {}

bool
Scheduler::addAgentToScheduler(Agent* agent) {
    if (agentMap[agent->getAgentID()] == NULL) {
        agentMap[agent->getAgentID()] = agent;
        agent->fibHeapPtr = reinterpret_cast<void *>(agentPQ.push(agent));
        return true;
    }
    return false;
}

void
Scheduler::updateKey(void* pointer, Time uTime) {
    AgentPQ::pointer ptr = reinterpret_cast<AgentPQ::pointer>(pointer);
    agentPQ.update(ptr, uTime);
}

bool
Scheduler::processNextAgentEvents() {
    if (agentPQ.top()->eventPQ->empty()) {
        return false;
    }
    
    Agent* agent = agentPQ.top();
    Time oldTopTime = agent->getTopTime();
    bool result = agent->processNextEvents();
    updateKey(agent->fibHeapPtr, oldTopTime);
    
    return result;
}

bool
Scheduler::scheduleEvent(Event* e) {
    // Make sure the recevier agent has an entry
    AgentID agent_id = e->getReceiverAgentID();
    AgentIDAgentPointerMap::iterator entry = agentMap.find(agent_id);
    Agent* agent = (entry != agentMap.end()) ? entry->second : NULL ;
    
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

    //will use this to figure out if we need to change our key in
    //scheduler
    Time oldTopTime = agent->getTopTime();
    
    // Process rollbacks if needed
    checkAndHandleRollback(e, agent);
    // If the event is an anti-message then all pending future events
    // from the sender should also be deleted.
    if (e->isAntiMessage()) {
        handleFutureAntiMessage(e, agent);
        updateKey(agent->fibHeapPtr, oldTopTime);
        return false;
    }
    
    ASSERT(e->isAntiMessage() == false);
    // Actually add the event (push increments reference count)
    agent->eventPQ->push(e);
    updateKey(agent->fibHeapPtr, oldTopTime);
    
    return true;
}

bool
Scheduler::checkAndHandleRollback(const Event* e, Agent* agent) {
    if (e->getReceiveTime() <= agent->getLVT()) {
        ASSERT(e->getSenderAgentID() != e->getReceiverAgentID());
        DEBUG(std::cout << "Rollingback due to: " << *e << std::endl);
        agent->doRollbackRecovery(e);
        if (e->getReceiveTime() <= agent->getLVT()) {
            // Error condition.
            std::cerr << "Rollback logic did not restore state correctly?\n";
            std::cerr << "Agent info:\n" << *agent << std::endl;
            abort();
        }
        return true;
    }
    return false;
}

void
Scheduler::handleFutureAntiMessage(const Event* e, Agent* agent){
    DEBUG(std::cout << "*Cancelling due to: " << *e << std::endl);
    // This event is an anti-message we must remove it and
    // future events from this agent.
    agent->eventPQ->removeFutureEvents(e);
    // There are cases when we may not have a future anti-message as
    // partial cleanup of input-queues done by
    // Agent::doCancellationPhaseInputQueue method may have already
    // removed this message.  Maybe there is a better way to rework
    // the whole input-queue handling in Agent to correctly detect and
    // report fast-anti messages.
}

Scheduler::~Scheduler() {}

Time
Scheduler::getNextEventTime() {
    // If the queue is empty, return infinity
    if (agentPQ.empty()) {
        return TIME_INFINITY;
    }
    // Otherwise, return the time of the top agent
    return agentPQ.top()->getTopTime();
}

#endif
