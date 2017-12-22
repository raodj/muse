#ifndef MUSE_MULTI_THREADED_SHM_SIMUALTION_CPP
#define MUSE_MULTI_THREADED_SHM_SIMUALTION_CPP

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

#include <string>
#include "mpi-mt-shm/MultiThreadedShmSimulationManager.h"
#include "mpi-mt-shm/MultiThreadedShmCommunicator.h"
#include "mpi-mt-shm/SingleBlockingMTQueueShm.h"
#include "GVTManager.h"
#include "GVTMessage.h"
#include "Scheduler.h"
#include "ArgParser.h"
#include "EventAdapter.h"

// Switch to muse namespace to streamline code
using namespace muse;

MultiThreadedShmSimulation::MultiThreadedShmSimulation(MultiThreadedShmSimulationManager* mgr,
                                                 int thrID, int globalThrID,
                                                 int threadsPerNode) :
    threadsPerNode(threadsPerNode), threadID(thrID),
    globalThreadID(globalThrID), mtCommMgr(NULL), simMgr(mgr) {
    ASSERT(mgr != NULL);
    ASSERT(threadsPerNode > 0);
    ASSERT(thrID >= 0);
    // Nothing much to be done for now as base class does all the
    // necessary work.
}

MultiThreadedShmSimulation::~MultiThreadedShmSimulation() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

void
MultiThreadedShmSimulation::setCommManager(MultiThreadedShmCommunicator* mtc) {
    // Save pointer to derived class instance so that we don't need to
    // do weird type casting.
    mtCommMgr   = mtc;
    // Set pointer in base class as well
    commManager = mtc;
    // Update the MPI information in the base class
    unsigned int totNumThreads;  // dummy. not really used.
    commManager->getProcessInfo(myID, numberOfProcesses, totNumThreads);
}

void
MultiThreadedShmSimulation:: initialize(int& argc, char* argv[], bool initMPI)
    throw(std::exception) {
    UNUSED_PARAM(initMPI);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);
}

void
MultiThreadedShmSimulation::finalize(bool stopMPI, bool delCommMgr) {
    // The base class does all the necessary work
    Simulation::finalize(stopMPI, delCommMgr);
    // Delete our incoming queue as we no longer need it.
    if (incomingEvents != NULL) {
        delete incomingEvents;
        incomingEvents = NULL;  // for sanity checks
    }
}

void
MultiThreadedShmSimulation::processIncomingEvents() {
    // Get all incoming events in an MT-safe manner.
    EventContainer eventList = incomingEvents->removeAll(threadID);
    // Process each event in a manner similar to base class operations
    for (Event* event : eventList) {
        if (event->getSenderAgentID() == -1) {
            // This must be a GVT message.
            ASSERT(event->getReceiverAgentID() <= 0);
            ASSERT(event->getReceiveTime() == -1);
            GVTMessage *msg = dynamic_cast<GVTMessage*>(event);
            ASSERT(msg != NULL);
            gvtManager->recvGVTMessage(msg);
        } else {
            // This is a regular event.  All incoming events must be
            // inspected by the GVT manager (for tracking GVT) prior
            // to further processing.
            gvtManager->inspectRemoteEvent(event);
            scheduleEvent(event);
            // Decrease the reference because if it was rejected,
            // the event will be properly deleted. However, if it
            // is in the eventPQ, it will be unharmed (reference
            // count will actually be fixed to correct for lack of
            // local output queue)
            ASSERT(EventRecycler::getReferenceCount(event) < 3);
            EventRecycler::decreaseReference(event);
        }
        // Note: Don't skip this step -- Let the GVT Manager
        // forward any pending control messages, if needed
        gvtManager->checkWaitingCtrlMsg();
    }
    if (eventList.empty()) {
        // Note: Don't skip this step -- Let the GVT Manager forward
        // any pending control messages, if needed
        gvtManager->checkWaitingCtrlMsg();
    }
}

void
MultiThreadedShmSimulation::simulate() {
    // Initialize all the agents
    initAgents();
    // Start the core simulation loop.
    LGVT         = startTime;
    int gvtTimer = gvtDelayRate;
    // The main simulation loop
    while (gvtManager->getGVT() < endTime) {
        // See if a stat dump has been requested
        if ((threadID == 0) && doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }
        if (--gvtTimer == 0) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        // Process a block of events received via the network (goes to
        // derived manager class).
        processMpiMsgs();
        // Process incoming events and update GVT token
        processIncomingEvents();
        // Process the next event from the list of events managed by
        // the scheduler.
        processNextEvent();
    }
}

bool 
MultiThreadedShmSimulation::scheduleEvent(Event* e) {
    ASSERT(e->getReceiveTime() >= getGVT());
    ASSERT((e->getReferenceCount() == 1) || (e->getReferenceCount() == 2));

    if (TIME_EQUALS(e->getSentTime(), TIME_INFINITY) ||
        (e->getSenderAgentID() == -1)) {
        std::cerr << "Don't use this method with a new event, go "
                  << "through the agent's scheduleEvent method." << std::endl;
        abort();
    }
    // Convenient reference to the destination agent ID
    const AgentID recvAgentID = e->getReceiverAgentID();
    // Find destination thread ID
    const int destThrID = mtCommMgr->getOwnerThreadRank(recvAgentID);
    ASSERT(destThrID != -1);
    if (destThrID == globalThreadID) {
        // Local events are directly inserted into our own scheduler
        return scheduler->scheduleEvent(e);
    } else {
        // Remote events are sent via the GVTManager to aid tracking
        // GVT. The gvt manager calls communicator.
        if (destThrID / threadsPerNode == (int) myID) {
            // Destination thread is on same process.  Since this
            // event is crossing thread boundaries we make a copy to
            // avoid race conditions on the reference counter.
            const int evtSize = EventAdapter::getEventSize(e);
            Event *copy = reinterpret_cast<Event*>(Event::allocate(evtSize));
            std::memcpy(copy, e, evtSize);
            ASSERT(copy->getReferenceCount() > 0);
            // copy->setReferenceCount(1);
            gvtManager->sendRemoteEvent(copy);
        } else {
            // The destination thread is on a remote process and event
            // does not need to be cloned.  However, it does need
            // inspection by GVT manager.
            gvtManager->sendRemoteEvent(e);
        }
    }
    return true;
}

void
MultiThreadedShmSimulation::parseCommandLineArgs(int &argc, char* argv[]) {
    // Make the arg_record
    std::string mtQueue = "single-blocking";
    ArgParser::ArgRecord arg_list[] = {
        { "--mt-queue", "MT-safe queue to use for events from other threads",
          &mtQueue, ArgParser::STRING },
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Setup the MT-queue for this process to use
    if (mtQueue == "single-blocking") {
        incomingEvents = new SingleBlockingMTQueueShm();
    } else {
        // Invalid mt-queue name.
        throw std::runtime_error("Invalid value for --mt-queue argument" \
                                 "(muse be: single-blocking");        
    }
    // Let base class process consume other arguments as appropriate
    Simulation::parseCommandLineArgs(argc, argv);
}

void
MultiThreadedShmSimulation::preStartInit() {
    // First let the base class do the necessary setup
    Simulation::preStartInit();
    // Now override the GVT manager's rank with thread-based rank
    ASSERT(gvtManager != NULL);
    gvtManager->setThreadedRank(globalThreadID);
}

int
MultiThreadedShmSimulation::processMpiMsgs() {
    ASSERT(simMgr != NULL);
    ASSERT(simMgr != this);  // Ensure no recursion
    // sub-threads delegate pumping MPI messages to the manager
    // thread.  This is necessary because only manager has pointers to
    // the list of threads so that events can be added to ther
    // incoming event queue(s).
    return simMgr->processMpiMsgs();
}

#endif
