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
//
//---------------------------------------------------------------------------

#include "Communicator.h"
#include "Simulation.h"
#include "GVTManager.h"
#include "HRMScheduler.h"
#include "SimulationListener.h"
#include "BinaryHeapWrapper.h"
#include "ArgParser.h"
#include "EventAdapter.h"
#include <cstdlib>
#include <csignal>
#include <unistd.h>

// The different types of simulators currently supported
#include "DefaultSimulation.h"
#include "mpi-mt/MultiThreadedSimulationManager.h"
// #include "mpi-mt-shm/MultiThreadedShmSimulationManager.h"

using namespace muse;

// Define reference to the singleton simulator/kernel
muse::Simulation* muse::Simulation::kernel = NULL;

Simulation::Simulation(const bool useSharedEvents)
    : LGVT(0), startTime(0), endTime(0), gvtDelayRate(10),
      numberOfProcesses(-1u), doShareEvents(useSharedEvents) {
    commManager        = NULL;
    scheduler          = NULL;
    myID               = -1u;
    listener           = NULL;
    doDumpStats        = false;
    mustSaveState      = false;
    maxMpiMsgThresh    = 1000;
    processMpiMsgCalls = 0;
    mpiMsgCheckThresh  = 1;
    mpiMsgCheckCounter = mpiMsgCheckThresh;
}

Simulation*
Simulation::initializeSimulation(int& argc, char* argv[], bool initMPI)
    throw (std::exception) {
    // First use a temporary argument parser to determine type of
    // simulation kernel to instantiate.
    std::string simName = "default";
    ArgParser::ArgRecord arg_list[] = {
        { "--simulator", "The type of simulator/kernel to use; one of: " \
          "default, mpi-mt, mpi-mt-shm", 
          &simName, ArgParser::STRING },
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Instantiate the actual simulation object based on simName.
    if (simName == "default") {
        kernel = new DefaultSimulation();
    } else if (simName == "mpi-mt") {
        kernel = new MultiThreadedSimulationManager();
    // } else if (simName == "mpi-mt-shm") {
    //     kernel = new MultiThreadedShmSimulationManager();
    } else {
        // Invalid simulator name.
        throw std::runtime_error("Invalid value for --simulator argument" \
                                 "(muse be: default, mpi-mt, or mpi-mt-shm)");
    }
    // Now let the instantiated/derived kernel initialize further.
    ASSERT (kernel != NULL);
    kernel->initialize(argc, argv, initMPI);  // can throw exception!
    // When control drops here, things went fine thus far
    return kernel;
}

void
Simulation::finalizeSimulation(bool stopMPI, bool delSim) {
    // First let the derived class finalize
    ASSERT( kernel != NULL );
    kernel->finalize(stopMPI);
    // If the simulator is to be deleted, do that now.
    if (delSim) {
        delete kernel;  // get rid of singleton instance.
        kernel = NULL;
    }
}

void
Simulation::initialize(int& argc, char* argv[], bool initMPI)
    throw (std::exception) {
    UNUSED_PARAM(argc);   // Suppress compiler warnings about these 3
    UNUSED_PARAM(argv);   // unused parameters
    UNUSED_PARAM(initMPI);
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
        { "--max-mpi-msg-thresh", "Maximum consecutive MPI msgs to process",
          &maxMpiMsgThresh, ArgParser::INTEGER},
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
    mustSaveState = (saveState || (numberOfProcesses > 1) ||
                     (getNumberOfThreads() > 1));
}


Simulation::~Simulation() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

bool
Simulation::registerAgent(muse::Agent* agent, const int threadRank)  {
    UNUSED_PARAM(threadRank);
    if (scheduler->addAgentToScheduler(agent)) {
        allAgents.push_back(agent);
        agent->mustSaveState = this->mustSaveState;
        agent->setKernel(this);
        return true;
    }
    return false;
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
Simulation::preStartInit() {
    ASSERT( commManager != NULL );
    // Create and initialize our GVT manager.
    gvtManager = new GVTManager(this);
    gvtManager->initialize(startTime, commManager);
    // Set gvt manager with the communicator.
    commManager->setGVTManager(gvtManager);
    // Inform scheduler(s) that simulation is starting.
    scheduler->start(startTime);
}

void
Simulation::initAgents() {
    // Loop for the initialization
    for (muse::Agent* agent : allAgents) {
        ASSERT( agent != NULL );
        agent->setLVT(startTime);  // Setup initial LVT
        agent->initialize();       // Call API method to initialize agent
        // time to archive the agent's init state
        agent->saveState();
    }        
}

int
Simulation::checkProcessMpiMsgs() {
    // A fixed threshold for sample count
    const int MpiThreshSamples = 100;
    // If we have only one process then there is nothing to be done
    if (numberOfProcesses < 2) {
        return 0;  // no other process to communicate with.
    }
    // Call the actual processMpiMsgs() method based on counter
    // settings.
    int numEvts = -1;
    if (--mpiMsgCheckCounter == 0) {
        numEvts = processMpiMsgs();
        if (numEvts == 0) {
            if ((mpiMsgCheckThresh == 1) &&
                (maxMpiMsgCheckThresh.getCount() > MpiThreshSamples)) {
                // Once we have collected sufficient samples we can
                // setup our threshold directly to that value.
                mpiMsgCheckThresh = maxMpiMsgCheckThresh.getMean();
            } else {
                mpiMsgCheckThresh = std::min(128, mpiMsgCheckThresh + 1);
            }
        } else {
            // Track the maximum value for mpiMsgCheckThresh for use
            // later on.
            maxMpiMsgCheckThresh += mpiMsgCheckThresh;
            mpiMsgCheckThresh     = 1;  // Reset back to 1.
        }
        mpiMsgCheckCounter = mpiMsgCheckThresh;
    }
    return numEvts;
}

int
Simulation::processMpiMsgs() {
    // If we have only one process then there is nothing to be done
    if (numberOfProcesses < 2) {
        return 0;  // no other process to communicate with.
    }
    // Track number of times the processMpiMsgs method is called.
    processMpiMsgCalls++;
    // An optimization trick is to try to get as many events from
    // the wire as we can. A good magic number is 100.  However
    // this number could be dynamically adapted depending on
    // behavior of the simulation.
    int numMsgs;
    for (numMsgs = 0; (numMsgs < maxMpiMsgThresh); numMsgs++) {
        Event* incoming_event = commManager->receiveEvent();
        if (incoming_event != NULL) {
            ASSERT(incoming_event->getReferenceCount() == 1);
            scheduleEvent(incoming_event);
            // Decrease the reference because if it was rejected,
            // the event will be properly deleted. However, if it
            // is in the eventPQ, it will be unharmed (reference
            // count will actually be fixed to correct for lack of
            // local output queue)
            ASSERT(EventRecycler::getReferenceCount(incoming_event) < 3);
            EventRecycler::decreaseReference(incoming_event);
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
            break;  // No message current available.
        }
        // Let the GVT Manager forward any pending control
        // messages, if needed
        gvtManager->checkWaitingCtrlMsg();
    }
    // Track the batch size if numMsgs is greater than zero.
    if (numMsgs > 0) {
        mpiMsgBatchSize += numMsgs;
    }
    return numMsgs;
}

bool
Simulation::processNextEvent() {
    // Update lgvt to the time of the next event to be processed.
    LGVT = scheduler->getNextEventTime();
    // Do sanity checks.
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
    // Let the scheduler do its task to have the agent process events
    // if possible.  If no events are processed the following method
    // call returns false.
    return scheduler->processNextAgentEvents();    
}

void 
Simulation::start() {
    // This check below should be removed -- If no agents registered
    // we need to leave start and end sim
    if (allAgents.empty()) return;
    // Finish all the setup prior to starting simulation.
    preStartInit();
    // Next initialize all the agents
    initAgents();
    // Start the core simulation loop.
    LGVT         = startTime;
    int gvtTimer = gvtDelayRate;
    // The main simulation loop
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
        // Process a block of events received via the network, while
        // performing exponential backoff as necessary.
        checkProcessMpiMsgs();
        // Process the next event from the list of events managed by
        // the scheduler.
        if (!processNextEvent()) {
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;
        }
    }
    // Wait for all the parallel processes to complete the main
    // simulation loop.
    MPI_BARRIER();
}

void
Simulation::finalize(bool stopMPI, bool delCommMgr) {
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
        // Remove agent from scheduler
        scheduler->removeAgentFromScheduler(agent);
        // Bye byte agent!
        delete agent;
    }

    // Now delete GVT manager as we no longer need it.
    commManager->setGVTManager(NULL);
    delete gvtManager;
    gvtManager = NULL;
    
    // Finalize the communicator 
    commManager->finalize(stopMPI);
    // Delete it if requested
    if (delCommMgr) {
        delete commManager;
    }
    commManager = NULL;

    // Invalidate the  kernel ID
    myID = -1u;

    DEBUG({
            if (logFile != NULL) {
                // Un-hijack cout
                std::cout.rdbuf(oldstream);
                // Get rid of the log file.
                delete logFile;
                logFile = NULL;
            }
    });
    // Get rid of scheduler as we no longer need it
    delete scheduler;
    scheduler = NULL;
    // Clear out the list of agents in this simulation
    allAgents.clear();
    // Finally clear out any pending events in the event recyler.
    EventRecycler::deleteRecycledEvents();
    // Clear out any pending states in the event recycler
    StateRecycler::deleteRecycledStates();
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
Simulation::isAgentLocal(AgentID id) const {
    return commManager->isAgentLocal(id);
}

bool
Simulation::isAgentLocal(const AgentID id1, const AgentID id2) const {
    return commManager->isAgentLocal(id1, id2);
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
	// Track total statistics
        totalRollbacks       += agent->numRollbacks;
        totalCommittedEvents += agent->numCommittedEvents;
        totalMPIMessages     += agent->numMPIMessages;
        totalScheduledEvents += agent->numScheduledEvents;
        totalSchedules       += agent->numSchedules;
    }

    // Place all the statistics into a string buffer for convenience.
    std::ostringstream stats;
    stats << "\nStats from Kernel ID   : " << myID
          << "\nNumber of agents       : " << allAgents.size()
          << "\nTotal schedules        : " << totalSchedules
          << "\nTotal Scheduled Events : " << totalScheduledEvents 
          << "\nTotal Committed Events : " << totalCommittedEvents
          << "\nTotal #rollbacks       : " << totalRollbacks
          << "\nTotal #MPI messages    : " << totalMPIMessages
          << "\n#process MPI msgs calls: " << processMpiMsgCalls
          << "\nMPI msg batch size     : " << mpiMsgBatchSize
          << "\nMax MPI msg check thres: " << maxMpiMsgCheckThresh
          << "\nAdaptive time window   : " << scheduler->adaptiveTimeWindow
          << std::endl;    
    // Let derived class(es) report statistics (if any)
    reportLocalStatistics(stats);
    // Finally, report statistics from the EventRecycler
    stats << EventRecycler::getStats();    
    // Have the scheduler (and event queue) report statistics (if any)
    scheduler->reportStats(stats);
    // Report statistics
    if (myID == ROOT_KERNEL) {
        os << stats.str();   // report local stats
        // Receive stats from each process and print it. Earlier we
        // used to print stats in a fixed order which *may* cause
        // issues with many processes.  So this loop processes
        // messages as and when they become available.
        for (size_t numLeft = numberOfProcesses; (numLeft > 1); numLeft--) {
            int recvRank;  // The rank of the sending process
            os << commManager->receiveMessage(recvRank);
        }
    } else {
        // Not root kernel. So send stats to the root kernel.
        commManager->sendMessage(stats.str(), ROOT_KERNEL);
    }
}

#endif
