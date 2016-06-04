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

#include <cstdlib>
#include "Scheduler.h"
#include "Simulation.h"
#include "LadderQueue.h"
#include "HeapEventQueue.h"
#include "TwoTierHeapEventQueue.h"
#include "ThreeTierHeapEventQueue.h"
#include "ArgParser.h"

using namespace muse;

Scheduler::Scheduler() : agentPQ(NULL), timeWindow(0) {}

bool
Scheduler::addAgentToScheduler(Agent* agent) {
    ASSERT(agent != NULL);
    if (agentMap[agent->getAgentID()] == NULL) {
        agentMap[agent->getAgentID()] = agent;
        agent->fibHeapPtr = agentPQ->addAgent(agent);
        return true;
    }
    return false;
}

void
Scheduler::updateKey(void* pointer, Time uTime) {
    UNUSED_PARAM(pointer);
    UNUSED_PARAM(uTime);
    ASSERT("Not implemented" == NULL);    
}

bool
Scheduler::withinTimeWindow(muse::Agent* agent,
                            const muse::Event* const event) const {
    ASSERT(event != NULL);
    UNUSED_PARAM(event);
    const Time gvt = Simulation::getSimulator()->getGVT();
    // return ((agent->getLVT() - gvt) > 604800000);
    return ((agent->getLVT() - gvt) <= timeWindow);
}

bool
Scheduler::processNextAgentEvents() {
    // If the event queue is empty, do no further operations.
    if (agentPQ->empty()) {
        return false;
    }
    // Get the first of next batch of events to be scheduled.
    const muse::Event* const front = agentPQ->front();
    ASSERT(front != NULL);
    // Figure out the agent to receive this event.
    Agent* const agent = agentMap[front->getReceiverAgentID()];
    ASSERT(agent != NULL);    
    // Check if the next lowest time-stamp event falls within time
    // window with respect to GVT.  If not, do not process events.
    if ((timeWindow > muse::Time(0)) && !withinTimeWindow(agent, front)) {
        return false;
    }
    // Have the next agent (with lowest receive timestamp events) to
    // process its batch of events.
    muse::EventContainer events;
    agentPQ->dequeueNextAgentEvents(events);
    agent->processNextEvents(events);
    // Processed some events
    return true;
}

bool
Scheduler::scheduleEvent(Event* e) {
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
    checkAndHandleRollback(e, agent);
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

bool
Scheduler::checkAndHandleRollback(const Event* e, Agent* agent) {
    if (e->getReceiveTime() <= agent->getLVT()) {
        ASSERT(e->getSenderAgentID() != e->getReceiverAgentID());
        DEBUG(std::cout << "Rollingback due to: " << *e << std::endl);
        // Have the agent to do inputQ, and outputQ clean-up and
        // return list of events to reschedule.
        muse::EventContainer reschedule;
        agent->doRollbackRecovery(e, *agentPQ);
        // Basic sanity check on correct rollback behavior
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
    agentPQ->eraseAfter(agent, e->getSenderAgentID(), e->getSentTime());
    // agent->eventPQ->removeFutureEvents(e);
    // There are cases when we may not have a future anti-message as
    // partial cleanup of input-queues done by
    // Agent::doCancellationPhaseInputQueue method may have already
    // removed this message.  Maybe there is a better way to rework
    // the whole input-queue handling in Agent to correctly detect and
    // report fast-anti messages.
}

Scheduler::~Scheduler() {
    if (agentPQ != NULL) {
        delete agentPQ;
    }
}

Time
Scheduler::getNextEventTime() {
    // If the queue is empty, return infinity
    if (agentPQ->empty()) {
        return TIME_INFINITY;
    }
    // Otherwise, return the time of the top agent
    return agentPQ->front()->getReceiveTime();
}

void
Scheduler::initialize(int rank, int numProcesses, int& argc, char* argv[])
    throw (std::exception) {
    UNUSED_PARAM(rank);
    UNUSED_PARAM(numProcesses);
    // Setup local variables for processing command-line arguments
    std::string queueName = "fibHeap";
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
        {"--scheduler-queue",
         "Queue (heap or fibHeap or ladderQ) to be used by scheduler",
         &queueName, ArgParser::STRING},
        {"--time-window", "Time window for scheduler to control optimism",
         &timeWindow, ArgParser::DOUBLE},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the argument parser to parse command-line arguments and
    // update local variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Create a queue based on the name specified.
    if (queueName == "heap") {
        agentPQ = new HeapEventQueue();
    } else if (queueName == "ladderQ") {
        agentPQ = new LadderQueue();
    } else if (queueName == "2tHeap") {
        agentPQ = new TwoTierHeapEventQueue(); 
    } else if (queueName == "3tHeap") {
        agentPQ = new ThreeTierHeapEventQueue();
    } else {
        agentPQ = new AgentPQ();
    }
}

void
Scheduler::reportStats(std::ostream& os) {
    agentPQ->reportStats(os);
}

#endif
