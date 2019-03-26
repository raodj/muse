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
//
//---------------------------------------------------------------------------

#include <cstdlib>

#include "Scheduler.h"
#include "Simulation.h"
#include "ArgParser.h"

#ifdef RB_STATS
#include <sstream>
std::ostringstream rbStats;
std::ofstream rbStatsFile;
#endif

using namespace muse;

// temp queue used to reduce allocate and deallocate overheads
thread_local muse::EventContainer Scheduler::agentEvents;

Scheduler::Scheduler() : agentPQ(NULL), timeWindow(0), adaptTimeWindow(false) {}

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

bool
Scheduler::removeAgentFromScheduler(Agent* agent) {
    ASSERT(agent != NULL);
    if (agentMap[agent->getAgentID()] != NULL) {
        agentPQ->removeAgent(agent);  // remove agent from scheduler.
        // Clear out agent entry in our internal look-up map.
        agentMap[agent->getAgentID()] = NULL;
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
                            const muse::Event* const event) {
    ASSERT(event != NULL);
    UNUSED_PARAM(agent);
    const Time gvt = Simulation::getSimulator()->getGVT();
    // If the next lowest-timestamp event is within time-window go
    // ahead and process it.
    // const Time timeDelta = agent->getLVT() - gvt;
    const Time timeDelta = event->getReceiveTime() - gvt;
    if (timeDelta <= timeWindow) {
        return true;
    } else if (adaptTimeWindow) {
        // Maybe the time window is a bit too small. Start increasing
        // time window to try and accommodate the next event.
        adaptiveTimeWindow += (timeDelta * 1);
        timeWindow = std::max(1.0, adaptiveTimeWindow.getMean());
    }
    return false;  // Current event outside time window. Do not schedule
}

AgentID
Scheduler::processNextAgentEvents() {
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
    Agent* const agent = agentMap[front->getReceiverAgentID()];
    ASSERT(agent != NULL);
    // Check if the next lowest time-stamp event falls within time
    // window with respect to GVT.  If not, do not process events.
    if ((timeWindow > muse::Time(0)) && !withinTimeWindow(agent, front)) {
        return InvalidAgentID;  // No events to schedule.
    }
    // Have the next agent (with lowest receive timestamp events) to
    // process its batch of events.
    agentPQ->dequeueNextAgentEvents(agentEvents);
    agent->processNextEvents(agentEvents);
    agentEvents.clear();
    // Processed some events
    return agent->myID;
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
    DEBUG(static Avg avgRbDist);
    if (e->getReceiveTime() <= agent->getLVT()) {
        ASSERT(e->getSenderAgentID() != e->getReceiverAgentID());
        DEBUG(std::cout << "Rollingback due to: " << *e
                        << " at LVT: " << agent->getLVT() << std::endl);
        // If adaptive time window is to be used, then update the time
        // based on the current GVT value.
        if (adaptTimeWindow) {
            const Time gvt    = Simulation::getSimulator()->getGVT();
            const Time rbDist = e->getReceiveTime() - gvt;
            adaptiveTimeWindow += rbDist;            
            DEBUG(avgRbDist += rbDist);
            if (adaptiveTimeWindow.getCount() > 100) {
                // Sufficient samples have been accumulated.  Set time
                // window estimate.
                timeWindow = std::max(1.0, adaptiveTimeWindow.getMean());
                DEBUG(
                      if (adaptiveTimeWindow.getCount() % 50000 == 0) {
                          std::cout << "RB timeWindow set to: "
                                    << adaptiveTimeWindow << ", rbDist = "
                                    << avgRbDist << std::endl;
                      });
            }
        }
        // Have the agent to do inputQ, and outputQ clean-up and
        // return list of events to reschedule.
#ifndef RB_STATS
        agent->doRollbackRecovery(e, *agentPQ);
#else
        // Code has been compiled to record Rollback statistics. So do
        // that now.
        const Time rbLen         = agent->getLVT() - e->getReceiveTime();
        const int statesCanceled = agent->doRollbackRecovery(e, *agentPQ);
        rbStats << agent->getAgentID()   << ","
                << agent->getLVT()       << ","
                << agent->numSchedules   << ","
                << e->getSenderAgentID() << ","
                << e->getSentTime()      << ","
                << e->getReceiveTime()   << ","
                << statesCanceled        << ","
                << rbLen                 << "\n";
        agent->numSchedules = 0;
        if (rbStats.tellp() > 4000) {
            const std::string stats = rbStats.str();
            rbStatsFile.write(stats.c_str(), stats.size());
            rbStats.str("");
        }
#endif  // RB_STATS
        
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
    // This event is an anti-message we must remove it and future
    // events from this agent.  Since the event cancellations are in
    // the future, they are not rollbacks (and hence we don't track
    // these as part of RB_STATS)
    agentPQ->eraseAfter(agent, e->getSenderAgentID(), e->getSentTime());
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
Scheduler::initialize(int rank, int numProcesses, int& argc, char* argv[]) {
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
        {"--2t-ladderQ-t2k", "Sub-buckets per bucket in 2-tier ladder queue",
         &TwoTierBucket::t2k, ArgParser::INTEGER},
        {"--lq-max-rungs", "Max rungs for ladderQ / 2tLadderQ",
         &LadderQueue::MaxRungs, ArgParser::INTEGER},
        {"--adapt-time-window", "Use adaptive time window to control optimism",
         &adaptTimeWindow, ArgParser::BOOLEAN},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the argument parser to parse command-line arguments and
    // update local variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Set MaxRungs to be the same for 2tLadderQ as well
    TwoTierLadderQueue::MaxRungs = LadderQueue::MaxRungs;
    // Create a queue based on the name specified.
    if (queueName == "heap") {
        agentPQ = new HeapEventQueue();
    } else if (queueName == "ladderQ") {
        agentPQ = new LadderQueue();
    } else if (queueName == "2tHeap") {
        agentPQ = new TwoTierHeapEventQueue(); 
    } else if (queueName == "heap2tQ") {
        agentPQ = new TwoTierHeapOfVectorsEventQueue();
    } else if (queueName == "3tHeap") {
        agentPQ = new ThreeTierHeapEventQueue(); 
    } else if (queueName == "fibHeap") {
        agentPQ = new AgentPQ();
    } else if (queueName == "2tLadderQ") {
        agentPQ = new TwoTierLadderQueue();
    } else {
        std::cerr << "Invalid scheduler queue name. Valid queue names are:\n"
                  << "\tladderQ fibHeap heap 2tHeap 3tHeap heap2tQ.\n"
                  << "Aborting.\n";
        std::abort();  // throw an exception instead?
    }
#ifdef RB_STATS
    std::string rbFileName = "rb_stats_rank_" + std::to_string(rank) +
        "_" + std::to_string(numProcesses) + ".csv";
    rbStatsFile.open(rbFileName, std::ios::app);
    rbStats << "#Agent,LVT,Schedules,Sender,SentTime,RecvTime,CyclesCancelled,RBLen\n";
#endif
}

void
Scheduler::reportStats(std::ostream& os) {
    agentPQ->reportStats(os);
#ifdef RB_STATS
    const std::string stats = rbStats.str();
    rbStatsFile.write(stats.c_str(), stats.size());
    rbStats.str("");    
#endif
}

void 
Scheduler::prettyPrint(std::ostream& os) const {
    ASSERT( agentPQ != NULL );
    agentPQ->prettyPrint(os);
}

#endif

