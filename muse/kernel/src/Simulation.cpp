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
#include "Scheduler.h"
#include "SimulationListener.h"
#include "BinaryHeapWrapper.h"
#include <cstdlib>

#include <csignal>

using namespace muse;

Simulation::Simulation() : LGVT(0), startTime(0), endTime(0),
                           gvtDelayRate(10), numberOfProcesses(-1u) {
    commManager = new Communicator();
    scheduler   = new Scheduler();
    myID        = -1u;
    listener    = NULL;
    doDumpStats = false;
}

void
Simulation::initialize(int argc, char* argv[]) {
    myID = commManager->initialize(argc, argv);
    commManager->getProcessInfo(myID, numberOfProcesses);
    
    // If debugging is enabled, hijack cout and redirect it to a
    // LogID.txt file
    DEBUG({
            char logFileName[128];
            sprintf(logFileName, "Log%u.txt", myID);
            logFile = new ofstream(logFileName);
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

Simulation::~Simulation() {
    delete scheduler;
    delete commManager;
    DEBUG(delete logFile);
}

bool
Simulation::registerAgent(muse::Agent* agent)  { 
    if (scheduler->addAgentToScheduler(agent)) {
        allAgents.push_back(agent);
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
        cerr << "Don't use this method with a new event, go "
             << "through the agent's scheduleEvent method." << endl;
        abort();
    }
    AgentID recvAgentID = e->getReceiverAgentID();
    
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
       
        // An optimization trick we learned from WARPED is to try to
        // get as many event from the wire as we can. A good magic
        // number is 1000
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
                    scheduler->agentPQ.prettyPrint(std::cout);
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
            std::cout << "LGVT = " << LGVT << " is below GVT: " << getGVT()
                      << " which is serious error. Scheduled agents: \n";
            scheduler->agentPQ.prettyPrint(std::cerr);
            std::cerr << "Rank " << myID << " Aborting.\n";
            std::cerr << std::flush;
            DEBUG(logFile->close());
            abort();
        }
        
        scheduler->processNextAgentEvents();
    }    
}

void
Simulation::finalize() {
    int totalRollbacks          = 0;
    int totalCommittedEvents    = 0;
    int totalMPIMessages        = 0;
    int totalScheduledEvents    = 0;
    AgentContainer::iterator it = allAgents.begin();
    for (; it != allAgents.end(); it++) {  
        (*it)->finalize();
        (*it)->garbageCollect(TIME_INFINITY);
        int outputQueueSize = (*it)->outputQueue.size();
        (*it)->cleanInputQueue();
        (*it)->cleanStateQueue();
        (*it)->cleanOutputQueue();
        
        totalRollbacks       += (*it)->numRollbacks;
        totalCommittedEvents += (*it)->numCommittedEvents + outputQueueSize;
        totalMPIMessages     += (*it)->numMPIMessages;
        totalScheduledEvents += (*it)->numScheduledEvents;
        
        delete (*it);  
    }
    
    std::cout << "\nKernel[" << myID
              << "]\n Total Scheduled Events[" << totalScheduledEvents 
              << "]\n Total Committed Events[" << totalCommittedEvents
              << "]\n Total rollbacks[" << totalRollbacks
              << "]\n Total MPI messages[" << totalMPIMessages
              << "]" << std::endl;

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
    for (AgentContainer::iterator it = allAgents.begin();
         (it != allAgents.end()); it++) {  
        (*it)->garbageCollect(getGVT());
    }
    // Let listener know garbage collection for a given GVT value has
    // been completed.
    if (listener != NULL) {
        listener->garbageCollectionDone(getGVT());
    }
}

bool
Simulation::isAgentLocal(AgentID id) { return commManager->isAgentLocal(id); }

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
Simulation::updateKey(void* pointer, Time uTime) {
    scheduler->updateKey(pointer, uTime);
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
    statsFile << "AgentID LVT InputQueueSize OutputQueueSize StateQueueSize"
              << " EventQueueSize" << std::endl;
    
    for (AgentContainer::iterator i = allAgents.begin();
         (i != allAgents.end()); i++) {
        // Get a pointer to the agent (to avoid confusion)
        Agent* agent = *i;
        statsFile << agent->getAgentID() << " "
                  << agent->lvt << " "
                  << agent->inputQueue.size() << " "
                  << agent->outputQueue.size() << " "
                  << agent->stateQueue.size() << " "
                  << agent->eventPQ->size() << std::endl;
    }
    statsFile.flush();
    statsFile.close();
    
}

#endif
