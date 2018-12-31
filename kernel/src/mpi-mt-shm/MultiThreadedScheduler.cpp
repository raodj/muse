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

void
MultiThreadedScheduler::initialize(int rank, int numProcesses, int& argc,
                                   char* argv[]) {
    UNUSED_PARAM(rank);
    UNUSED_PARAM(numProcesses);
    std::string queueName = "3tSkipMT";
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
        {"--scheduler-queue",
         "Queue (3tSkipMT) to be used by multi threaded scheduler",
         &queueName, ArgParser::STRING},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the argument parser to parse command-line arguments and
    // update local variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Set MaxRungs to be the same for 2tLadderQ as well
    TwoTierLadderQueue::MaxRungs = LadderQueue::MaxRungs;
    // Create a queue based on the name specified.
    if (queueName == "3tSkipMT") {
        // set both priority queues here with the same reference.
        mtAgentPQ = new ThreeTierSkipMTQueue();
        agentPQ   = mtAgentPQ;
    } else {
        std::cerr << "Invalid scheduler queue name. Valid MT queue names are:\n"
                  << "\t(3tSkipMT).\n"
                  << "Aborting.\n";
        std::abort();  // throw an exception instead?
    }
    
    ASSERT(mtAgentPQ == agentPQ);
    
    Scheduler::initialize(rank, numProcesses, argc, argv);
}

bool
MultiThreadedScheduler::processNextAgentEvents(Time& simLGVT) {
    
    // === Dequeue next event set in thread safe way ===
    
    // This removes the agent from the priority queue, preventing any
    // other thread from dequeuing events on it
    muse::Agent *agent = mtAgentPQ->popNextAgent();
    // ******** THE ABOVE AGENT MUST BE PUT BACK BEFORE EXITING METHOD *********
    // (deperomm): bad practice above, need to fix, how?
    
    // this should only happen if all agents are popped
    // not enough agents in this sim for the number of threads being used?
    ASSERT(agent != NULL); 
    
    // Have the next agent (with lowest receive timestamp events) to
    // process its batch of events.
    // Do this in one method call so as to avoid race conditions
    mtAgentPQ->dequeueNextEvents(agent, agentEvents);
    
    // todo(deperomm): anti message support
    // if we dequeue an anti message, we have no choice but to ignore it for
    // now. Anti-messages are time sensitive, and must be handled immediately
    // upon being received in order to clean the queue of invalid events before
    // future valid events can be added, otherwise we have no way to
    // differentiate between events that came before or after the anti message
    // The solution to this would be to implement an epoch system where events
    // include an epoch num, and anti messages can includes a new epoch number
    // such that only events before that epoch would be invalid, so we know
    // which future events to delete when we dequeue an anti message
    size_t index = 0;
    while (!agentEvents.empty() && (index < agentEvents.size())) {
        Event* const evt = agentEvents[index];
        ASSERT(evt != NULL);
        // (deperomm): also assert that all events are valid and same time?
        if (evt->isAntiMessage()) {
            // Clean-up any pending future events (from the sender of the
            // anti-message) in the scheduler's queue.
//            handleFutureAntiMessage(e, agent); // this needs an epoch to work
            
            // (deperomm): for now, just delete the anti messages...
            agentEvents[index] = agentEvents.back();
            agentEvents.pop_back();
//            std::cout << "Agent " << agent->getAgentID() << " ignored anti message" << std::endl;
        } else {
            index++;
        }
    }

    
    // If the event queue is empty, do no further operations.
    if (agentEvents.empty()) {
        // This thread has no more events to process, LGVT = infinity right now
        simLGVT = TIME_INFINITY;
        // put the agent back
        mtAgentPQ->pushAgent(agent);
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
    
    // Since we're running concurrently, check for rollbacks on dequeue
    if(checkAndHandleRollback(front, agent)) {
        // put the events we pulled out back since we needed to roll back and
        // time priority has changed
        agentPQ->enqueue(agent, agentEvents);
        // put the agent back
        mtAgentPQ->pushAgent(agent);
        // pretend like we processed events so the sim doesn't think we're
        // out of events to process 
        // (deperomm): hacky solution, but not worth the rework to change
        // how return is handled
        return true;
    }
    
    // Check if the next lowest time-stamp event falls within time
    // window with respect to GVT.  If not, do not process events.
    if ((timeWindow > muse::Time(0)) && !withinTimeWindow(agent, front)) {
        // (deperomm) Doesn't this result in lost events? sholdn't agentEvents be rescheduled and the container cleared?
        // put agent back
        mtAgentPQ->pushAgent(agent);
        return false;  // No events to schedule.
    }
    
    // Let the agent process its events and then release its lock
    agent->processNextEvents(agentEvents);
    
    // put the agent back now that we're done with it
    mtAgentPQ->pushAgent(agent);
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

    // Actually add the event (enqueue indirectly increments reference count)
    agentPQ->enqueue(agent, e);
    // Event has been enqueued.
    return true;
}

END_NAMESPACE(muse)

#endif
