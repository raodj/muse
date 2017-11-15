#ifndef MUSE_MULTI_THREADED_SHM_SIMUALTION_MANAGER_CPP
#define MUSE_MULTI_THREADED_SHM_SIMUALTION_MANAGER_CPP

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

#include <thread>
#include <algorithm>
#include "mpi-mt-shm/MultiThreadedShmSimulationManager.h"
#include "mpi-mt-shm/MultiThreadedShmCommunicator.h"
#include "GVTMessage.h"
#include "ArgParser.h"

// Switch to muse namespace to streamline code
using namespace muse;

MultiThreadedShmSimulationManager::MultiThreadedShmSimulationManager()
    : MultiThreadedShmSimulation(this) {   // note -- NULL is intentional
    // Nothing much to be done for now as base class does all the
    // necessary work.
}

MultiThreadedShmSimulationManager::~MultiThreadedShmSimulationManager() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

void
MultiThreadedShmSimulationManager::initialize(int& argc, char* argv[],
                                           bool initMPI)
    throw (std::exception) {
    // First, parse out the number of threads to be created.
    ArgParser::ArgRecord arg_list[] = {
        { "--threads-per-node", "Number of threads to start per node/process",
          &threadsPerNode, ArgParser::INTEGER},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and extract the threads-per-node
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    ASSERT( threadsPerNode > 0 );

    // Next, initialize the communicator shared by multiple threads
    MultiThreadedShmCommunicator* mtc =
        new MultiThreadedShmCommunicator(this, threadsPerNode);
    // Let comm-manager use command-line arguments to configure itself.
    mtc->initialize(argc, argv, initMPI);
    setCommManager(mtc);  // set comm-manager for 'this' object.
    // Now create the necessary number of threads and initialize them.
    // To repeatedly and consistently initialize each thread class,
    // the command-line arguments are saved and restored.
    std::vector<char*> cmdArgs(argv, argv + argc);
    // First initialize & setup this class that runs as thread zero
    MultiThreadedShmSimulation::initialize(argc, argv, initMPI);
    // Only the manager explicitly initializes parent class to hijack streams
    Simulation::initialize(argc, argv, initMPI);
    // Setup our global thread ID
    globalThreadID = myID * threadsPerNode;
    // Add this class as thread zero.
    threads.push_back(this);
    // Create the necessary number of threads.
    for (int thrID = 1; (thrID < threadsPerNode); thrID++) {
        const int globalThrID = (myID * threadsPerNode) + thrID;
        MultiThreadedShmSimulation* tsm =
            new MultiThreadedShmSimulation(this, thrID, globalThrID,
                                        threadsPerNode);
        // Setup the pointer to shared comm-manager to be used
        tsm->setCommManager(mtc);        
        // Setup command-line arguments from a copy to preserve original
        argc = cmdArgs.size();
        std::vector<char*> cmdArgsCopy = cmdArgs;
        argv = cmdArgsCopy.data();
        // Let the thread initialize itself based on parameters.
        tsm->initialize(argc, argv, false);
        // Add the newly created thread to the list
        threads.push_back(tsm);
    }
    // Finally, let the base-class perform generic initialization
    muse::Simulation::initialize(argc, argv, initMPI);
}

bool
MultiThreadedShmSimulationManager::registerAgent(Agent* agent,
                                              const int threadRank) {
    // A static variable to persist across invocations to handle
    // round-robin assignment to differen threads.
    static int nextThreadIdx = 0;
    // Do basic checks.
    ASSERT(agent != NULL);
    ASSERT((threadRank >= -1) && (threadRank < threadsPerNode));

    // Setup the rank of the thread with which the agent should register
    int thrIdx = threadRank;
    if (thrIdx == -1) {
        // Do round-robin assignment to threads in this case
        thrIdx = nextThreadIdx;
        // Update index of next thread (for next time this method is called)
        nextThreadIdx = (nextThreadIdx + 1) % threadsPerNode;
    }
    // Register agent--thread mapping with the communication manager
    ASSERT(mtCommMgr != NULL);
    mtCommMgr->setAgentThread(agent->getAgentID(), thrIdx);
    // Register agent with the appropriate thread.
    if (thrIdx == 0) {
        // Register on this thread via base class method.
        return Simulation::registerAgent(agent);
    }
    // Register with another thread.
    return threads[thrIdx]->registerAgent(agent);
}

void
MultiThreadedShmSimulationManager::preStartInit() {
    ASSERT( commManager != NULL );
    // Let base class method do initial setup
    MultiThreadedShmSimulation::preStartInit();
    // Have all the threads do their pre-start init
    for (size_t thrIdx = 1; (thrIdx < threads.size()); thrIdx++) {
        ASSERT(threads[thrIdx] != NULL);
        threads[thrIdx]->preStartInit();
    }
    // Finally let the comm-manager finish exchanging list of agents
    // on different MPI processes which is needed to enable exchange
    // of events.
    mtCommMgr->registerAgents();
}

void
MultiThreadedShmSimulationManager::start() {
    // Finish all the setup for all the threads prior to starting simulation.
    preStartInit();
    // Now spwan threadsPerNode - 1 threads (this is thread #0 already)
    ASSERT( threadsPerNode > 0 );
    std::vector<std::thread> thrList;
    for (int thrIdx = 1; (thrIdx < threadsPerNode); thrIdx++) {
        // Setup most up to date parameter/options
        threads[thrIdx]->setStartTime(startTime);
        threads[thrIdx]->setStopTime(endTime);
        // Spin-up the thread.
        thrList.push_back(std::thread(&MultiThreadedShmSimulation::simulate,
                                      threads[thrIdx]));
    }
    // Now perform the simulation tasks for this thread #0
    simulate();
    // Now that the simulation is done, wind-up the threads
    std::for_each(thrList.begin(), thrList.end(),
                  std::mem_fn(&std::thread::join));
}

void
MultiThreadedShmSimulationManager::finalize(bool stopMPI, bool delCommMgr) {
    // First let all the sub-threads finalize
    for (size_t thrIdx = 1; (thrIdx < threads.size()); thrIdx++) {
        threads[thrIdx]->finalize(false, false);
    }
    // Next, finalize this thread #0
    MultiThreadedShmSimulation::finalize(stopMPI, delCommMgr);
    // Next get rid of all the thread helper classes as they are no
    // longer needed.  The thread #0 will be deleted in
    // Simulation::finalizeSimulation() method if user requests it.
    for (size_t thrIdx = 1; (thrIdx < threads.size()); thrIdx++) {
        delete threads[thrIdx];
    }
    threads.clear();  // Clear out the vector as a sanity check.
}

void
MultiThreadedShmSimulationManager::processMpiMsgs() {
    // If we have only one process then there is nothing to be done
    if (numberOfProcesses < 2) {
        return;  // no other process to communicate with.
    }
    // First try to lock the shared MPI mutex.  If it is not lockable,
    // then some other thread is pumping MPI messages for all
    // threads. So skip rest of the operation.
    if (!mpiMutex.try_lock()) {
        return;  // Some other thread is working.
    }
    // We got the lock read MPI messages if any.
    std::lock_guard<std::mutex> lock(mpiMutex, std::adopt_lock);
    // An optimization trick is to try to get as many events from the
    // wire as we can. A good magic number is 100.  However this
    // number could be dynamically adapted depending on behavior of
    // the simulation.
    for (int numMsgs = maxMpiMsgThresh; (numMsgs > 0); numMsgs--) {
        Event* incoming_event = commManager->receiveEvent();
        if (incoming_event != NULL) {
            ASSERT(incoming_event->getReferenceCount() == 1);
            const AgentID receiver = incoming_event->getReceiverAgentID();
            // Enqueue the incoming event to the appropriate thread.
            if (incoming_event->getSenderAgentID() == -1) {
                // This must be a GVT message.
                ASSERT(receiver <= 0);
                ASSERT(dynamic_cast<GVTMessage*>(incoming_event) != NULL);
                const unsigned int glblThrId = -receiver;
                DEBUG(std::cout << "myID = " << myID
                                << ", glblThrId = " << glblThrId << std::endl);
                ASSERT(glblThrId >= (myID + 0) * threadsPerNode);
                ASSERT(glblThrId <  (myID + 1) * threadsPerNode);
                // Always send incoming GVT messages to thread index #0.
                addIncomingEvent(glblThrId % threadsPerNode, incoming_event);
            } else {
                // Get the thread ID for the receiver agent.
                const int thrIdx = mtCommMgr->getThreadID(receiver);
                ASSERT( thrIdx != -1 );
                addIncomingEvent(thrIdx, incoming_event);
            }
        } else {
            // Incoming event was NULL. Nothing more to read for now.
            break;
        }
    }
}

#endif
