#ifndef MUSE_SIMUALTION_CPP
#define MUSE_SIMUALTION_CPP

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

#include "Communicator.h"
#include "Simulation.h"
#include "GVTManager.h"
#include "HRMScheduler.h"
#include "SimulationListener.h"
#include "BinaryHeapWrapper.h"
#include "ArgParser.h"
#include <cstdlib>
#include <csignal>

using namespace muse;

Simulation::Simulation() : LGVT(0), startTime(0), endTime(0),
                           gvtDelayRate(10), numberOfProcesses(-1u) {
    commManager   = new Communicator();
    scheduler     = NULL;
    myID          = -1u;
    listener      = NULL;
    doDumpStats   = false;
    mustSaveState = false;
}

void
Simulation::initialize(int& argc, char* argv[]) throw (std::exception) {
    myID = commManager->initialize(argc, argv);
    commManager->getProcessInfo(myID, numberOfProcesses);
    
    // If debugging is enabled, hijack cout and redirect it to a
    // LogID.txt file
    DEBUG({
            char logFileName[128];
            sprintf(logFileName, "Log%u.txt", myID);
            logFile = new std::ofstream(logFileName);
            oldstream = std::cout.rdbuf(logFile->rdbuf());
        });
    
    // Register a signal handler on SIGUSR1 to trap "dump" event. Use
    // sigaction() instead of signal() as per the signal(2) manpage.
    struct sigaction dumpStatAction;
    dumpStatAction.sa_handler = Simulation::dumpStatsSignalHandler;
    sigemptyset(&dumpStatAction.sa_mask);
    dumpStatAction.sa_flags = 0;
    sigaction(SIGUSR1, &dumpStatAction, NULL);
    sigaction(SIGUSR2, &dumpStatAction, NULL);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);
}

void
Simulation::parseCommandLineArgs(int &argc, char* argv[]) {
    // Make the arg_record
    bool saveState = false;
    ArgParser::ArgRecord arg_list[] = {
        { "--scheduler", "The scheduler to use (default or hrm)",
          &schedulerName, ArgParser::STRING },
        { "--gvt-delay", "Number of event cyles after  which to start "
          "GVT measurement", &gvtDelayRate, ArgParser::INTEGER},
        { "--save-state", "Force state saving (used only with 1 process)",
          &saveState, ArgParser::BOOLEAN},        
        {"", "", NULL, ArgParser::INVALID}
    };

    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Setup scheduler based on scheduler name.
    scheduler = (schedulerName == "hrm") ? new HRMScheduler() : new Scheduler();
    // Initialize the scheduler.
    scheduler->initialize(myID, numberOfProcesses, argc, argv);
    // Setup flag to enable/disable state saving in agents
    mustSaveState = (saveState || (numberOfProcesses > 1));
}


Simulation::~Simulation() {
    delete scheduler;
    delete commManager;
    DEBUG(delete logFile);
}

bool
Simulation::registerAgent(muse::Agent* agent)  { 
    if (scheduler->addAgentToScheduler(agent)) {
        allAgents.push_back(agent);
        agent->mustSaveState = this->mustSaveState;
        return true;
    }
    return false;
}

Simulation*
Simulation::getSimulator() {
    static Simulation kernel;
    return &kernel;
}

bool 
Simulation::scheduleEvent(Event* e) {
    ASSERT(e->getReceiveTime() >= getGVT());
    ASSERT(e->getReferenceCount() == 1);
    
    if (TIME_EQUALS(e->getSentTime(), TIME_INFINITY) ||
        (e->getSenderAgentID() == -1)) {
        std::cerr << "Don't use this method with a new event, go "
                  << "through the agent's scheduleEvent method." << std::endl;
        abort();
    }
    const AgentID recvAgentID = e->getReceiverAgentID();
    
    if (isAgentLocal(recvAgentID)) {
        // Local events are directly inserted into our own scheduler
        return scheduler->scheduleEvent(e);
    } else {
        // Remote events are sent via the GVTManager to aid tracking
        // GVT. The gvt manager calls communicator.
        gvtManager->sendRemoteEvent(e);
    }
    return true;
}

void 
Simulation::start() {
    //if no agents registered we need to leave start and end sim
    if (allAgents.empty()) return;
    //first we setup the AgentMap for all kernels
    commManager->registerAgents(allAgents);
    // Create and initialize our GVT manager.
    gvtManager = new GVTManager();
    gvtManager->initialize(startTime, commManager);
    // Set gvt manager with the communicator.
    commManager->setGVTManager(gvtManager);
    // Inform scheduler(s) that simulation is starting.
    scheduler->start(startTime);
    //information about the simulation environment
    unsigned int rank = -1u;
    commManager->getProcessInfo(rank, numberOfProcesses);
    
    //loop for the initialization
    AgentContainer::iterator it;
    for (it = allAgents.begin(); (it != allAgents.end()); it++) {
        (*it)->initialize();
        // time to archive the agent's init state
        (*it)->saveState();
        
    }
    
    LGVT         = startTime;
    int gvtTimer = gvtDelayRate;
    
    while (gvtManager->getGVT() < endTime) {
        // See if a stat dump has been requested
        if (doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }
        if (--gvtTimer == 0 ) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
       
        // An optimization trick is to try to get as many events from
        // the wire as we can. A good magic number is 1000.  However
        // this number should be dynamically adapted depending on
        // behavior of the simulation.
        for (int magic = 0; (magic < 1000); magic++) {
            Event* incoming_event = commManager->receiveEvent();
            if (incoming_event != NULL) {
                ASSERT(incoming_event->getReferenceCount() == 1);
                scheduleEvent(incoming_event);
                // Decrease the reference because if it was rejected,
                // the event will be properly deleted. However, if it
                // is in the eventPQ, it will be unharmed (reference
                // count will actually be fixed to correct for lack of
                // local output queue)
                ASSERT(incoming_event->getReferenceCount() < 3);
                incoming_event->decreaseReference();
                if ((LGVT = scheduler->getNextEventTime()) < getGVT()) {
                    std::cout << "LGVT = " << LGVT << " is below GVT: "
                              << getGVT()
                              << " which is serious error. "
                              << "Scheduled agents:\n";
                    scheduler->agentPQ->prettyPrint(std::cout);
                    std::cerr << "Rank " << myID << " Aborting.\n";
                    std::cerr << std::flush;
                    DEBUG(logFile->close());
                    ASSERT ( false );
                }
            } else {
                break; 
            }
            // Let the GVT Manager forward any pending control
            // messages, if needed
            gvtManager->checkWaitingCtrlMsg();
        }        
        
        // Update lgvt to the time of the next event to be processed.
        LGVT = scheduler->getNextEventTime();
        if (LGVT < getGVT()) {
            std::cout << "Offending event: "
                      << *scheduler->agentPQ->front() << std::endl;
            std::cout << "LGVT = " << LGVT << " is below GVT: " << getGVT()
                      << " which is serious error. Scheduled agents: \n";
            scheduler->agentPQ->prettyPrint(std::cout);
            std::cout << "Rank " << myID << " Aborting.\n";
            std::cout << std::flush;
            DEBUG(logFile->close());
            abort();
        }
        
        scheduler->processNextAgentEvents();
    }    
}

void
Simulation::finalize() {
    // Inform the scheduler that the simulation is complete
    scheduler->stop();
    // Finalize all the agents on this MPI process while accumulating stats
    for (AgentContainer::iterator it = allAgents.begin();
	 it != allAgents.end(); it++) {
	Agent* const agent = *it;
        agent->finalize();
        agent->garbageCollect(TIME_INFINITY);
        agent->cleanStateQueue();
        agent->cleanInputQueue();
        // Don't clean output queue yet as we need stats from it.
    }
    // Report aggregate statistics from this kernel
    reportStatistics();

    // Clean up all the agents
    for (AgentContainer::iterator it = allAgents.begin();
	 it != allAgents.end(); it++) {
        Agent* const agent = *it;
        agent->cleanOutputQueue();
        // Bye byte agent!
        delete agent;  
    }

    // Now delete GVT manager as we no longer need it.
    commManager->setGVTManager(NULL);
    delete gvtManager;
    gvtManager = NULL;
    
    // Finalize the communicator
    commManager->finalize();

    // Invalidate the  kernel ID
    myID = -1u;
    
    // Un-hijack cout
    DEBUG(std::cout.rdbuf(oldstream));
}

void
Simulation::garbageCollect() {
    const Time gvt = getGVT();
    // First let the scheduler know it can garbage collect.
    scheduler->garbageCollect(gvt);
    for (AgentContainer::iterator it = allAgents.begin();
         (it != allAgents.end()); it++) {  
        (*it)->garbageCollect(gvt);
    }
    // Let listener know garbage collection for a given GVT value has
    // been completed.
    if (listener != NULL) {
        listener->garbageCollectionDone(gvt);
    }
}

bool
Simulation::isAgentLocal(AgentID id) {
    return commManager->isAgentLocal(id);
}

void Simulation::stop() {}

Time
Simulation::getLGVT() const {
    return LGVT;
}

Time
Simulation::getGVT() const {
    return gvtManager->getGVT();
}

/*void
Simulation::updateKey(void* pointer, Time uTime) {
    scheduler->updateKey(pointer, uTime);
}
*/

void
Simulation::setListener(SimulationListener *callback) {
    listener = callback;
}

void
Simulation::dumpStatsSignalHandler(int sigNum) {
    // This is a static method in Simulation, which means we need to
    // get a reference to ourselves.
    Simulation* sim = Simulation::getSimulator();
    // If we get USR1, this is a lazy (safe) dump. If we get USR, it's
    // an emergency (unsafe) dump
    if (sigNum == SIGUSR1) {
        // Tell the simulator to dump stats on the next cycle
        sim->doDumpStats = true;
    } else if (sigNum == SIGUSR2) {
        sim->dumpStats();
    }

}
void
Simulation::dumpStats() {
    // Figure out a file name for these statistics
    char statsFileName[128];
    sprintf(statsFileName, "Stats%u.txt", myID);
    std::ofstream statsFile(statsFileName, std::ios::app);
    statsFile.precision(20);
    // Now that we have a file output stream, let's print a header.
    statsFile << "# GVT: " << gvtManager->getGVT()
              << ", LGVT: " << LGVT << std::endl;
    // Have all the agents dump their stats to the file.
    for (AgentContainer::iterator agentIter = allAgents.begin();
         (agentIter != allAgents.end()); agentIter++) {
        (*agentIter)->dumpStats(statsFile, agentIter == allAgents.begin());
    }
    statsFile.close();
}

void
Simulation::reportStatistics(std::ostream& os) {
    int totalRollbacks       = 0;
    int totalCommittedEvents = 0;
    int totalMPIMessages     = 0;
    int totalScheduledEvents = 0;
    int totalSchedules       = 0;

    // Collect stats from all the agents on this MPI process
    for (AgentContainer::iterator it = allAgents.begin();
	 it != allAgents.end(); it++) {
	Agent* const agent = *it;
        const int outputQueueSize = agent->outputQueue.size();
	// Track total statistics
        totalRollbacks       += agent->numRollbacks;
        totalCommittedEvents += agent->numCommittedEvents + outputQueueSize;
        totalMPIMessages     += agent->numMPIMessages;
        totalScheduledEvents += agent->numScheduledEvents;
        totalSchedules       += agent->numSchedules;
    }

    // Place all the statistics into a string buffer for convenience.
    std::ostringstream stats;
    stats << "\nStats from Kernel ID  : " << myID
          << "\nNumber of agents      : " << allAgents.size()
          << "\nTotal schedules       : " << totalSchedules
          << "\nTotal Scheduled Events: " << totalScheduledEvents 
          << "\nTotal Committed Events: " << totalCommittedEvents
          << "\nTotal #rollbacks      : " << totalRollbacks
          << "\nTotal #MPI messages   : " << totalMPIMessages
          << std::endl;
    // Have the scheduler (and event queue) report statistics (if any)
    scheduler->reportStats(stats);
    // Report statistics
    if (myID == ROOT_KERNEL) {
        os << stats.str();   // report local stats
        for (size_t rank = 1; (rank < numberOfProcesses); rank++) {
            int recvRank;
            os << commManager->receiveMessage(recvRank, rank);
            ASSERT( recvRank == static_cast<int>(rank) );
        }
    } else {
        // Not root kernel. So send stats to the root kernel.
        commManager->sendMessage(stats.str(), ROOT_KERNEL);
    }
}

#endif
