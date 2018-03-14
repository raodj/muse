#ifndef MUSE_MULTI_THREADED_SHM_COMMUNICATOR_CPP
#define MUSE_MULTI_THREADED_SHM_COMMUNICATOR_CPP

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "mpi-mt-shm/MultiThreadedShmCommunicator.h"
#include "mpi-mt-shm/MultiThreadedShmSimulationManager.h"
#include "GVTManager.h"
#include "GVTMessage.h"
#include "DataTypes.h"
#include "Event.h"
#include "Agent.h"

using namespace muse;

MultiThreadedShmCommunicator::MultiThreadedShmCommunicator(MultiThreadedShmSimulationManager* simMgr,
                                                     const int thrPerNode)
    : simMgr(simMgr), threadsPerNode(thrPerNode) {
    ASSERT( simMgr != NULL );
    ASSERT( threadsPerNode > 0 );
    // Nothing else to be done here for now
}

MultiThreadedShmCommunicator::~MultiThreadedShmCommunicator() {
    // Nothing else to be done here for now
}

SimulatorID
MultiThreadedShmCommunicator::initialize(int argc, char* argv[],
                                      bool initMPI) {
    // The base class does the necessary initialization
    SimulatorID simID = Communicator::initialize(argc, argv, initMPI);
    // Finally return the simulation ID from base class
    return simID;
}

void
MultiThreadedShmCommunicator::finalize(bool stopMPI) {
    // For now the base class does all the necessary work.
    Communicator::finalize(stopMPI);
}

void
MultiThreadedShmCommunicator::getProcessInfo(unsigned int& rank,
                                          unsigned int& numProcesses,
                                          unsigned int& totNumThreads) {
    // Get basic information from parent process
    Communicator::getProcessInfo(rank, numProcesses, totNumThreads);
    // Update the total number of threads

    // todo (deperomm): Hack, mpi-mt thinks processes are threads, so for true shared memory parallization, leave num of threads as the num of processes
    // Uncomment out line below once hack is fixed (mpi-mt knows difference between threads/processes)
    // as only reason this is possible is becasue totNumThreads is not used anywhere else in mpi-mt-shm
    // Other part of this hack is located in GVTManager.cpp, search deperomm for relavent comment block
    // totNumThreads = numProcesses * threadsPerNode;
}

void
MultiThreadedShmCommunicator::sendEvent(Event* e, const int eventSize){
    // Do some basic sanity checks.
    ASSERT( e != NULL );
    // This should only be used for remote events, as local events are handled
    // internally with a shared event queue
    ASSERT( getOwnerRank(e->getReceiverAgentID()) != 
            getOwnerRank(e->getSenderAgentID()) );
    ASSERT( !isAgentLocal(e->getReceiverAgentID()) );
    std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
    Communicator::sendEvent(e, eventSize);
}

void
MultiThreadedShmCommunicator::sendMessage(const GVTMessage *msg,
                                       const int destRank) {
    ASSERT(msg != NULL);
    ASSERT(msg->getSenderAgentID() == -1);
    ASSERT(msg->getReceiverAgentID() == -destRank);
    
    // First check if this message needs to go another thread on the
    // same node.
    const unsigned int rank = destRank / threadsPerNode;
    if (rank == myMPIrank) {
        // This message must go to next logical thread.  Since it is
        // going across thread boundaries, a copy needs to be made.
        // Set destination rank as negative value to help reciever
        // (see MultiThreadedSimulationManager::processMpiMsgs) to
        // detect & route message to appropriate thread
        GVTMessage* copy = GVTMessage::create(msg, -destRank,
                                              destRank % threadsPerNode);
        // simMgr->processIncomingEvent(copy); // Matt: This shouldn't happen, should be handled by the MTSimulation level, intentionally results in error as method is protected, part of event managemetn rework
    } else {
        // Msg needs to be sent to a remote thread.
        // Let the base class dispatch it over MPI in a MT-safe
        // operations.
        // todo(deperomm): This lock seems like it could be a big bottleneck in
        // multi threaded sims as every remote event must lock to send via mpi?
        std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
        Communicator::sendMessage(msg, rank);
    }
}

void
MultiThreadedShmCommunicator::sendMessage(const std::string& str,
                                       const int destRank, int tag) {
    // Ensure MT-safe MPI calls
    std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
    // Let base class do the actual operation.
    Communicator::sendMessage(str, destRank, tag);
}

std::string
MultiThreadedShmCommunicator::receiveMessage(int& recvRank, const int srcRank,
                                          int tag, bool blocking) {
    // Ensure MT-safe MPI calls
    std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
    // Let base class do the actual operation.
    return Communicator::receiveMessage(recvRank, srcRank, tag, blocking);
}

Event*
MultiThreadedShmCommunicator::receiveEvent() {
    // Ensure MT-safe MPI calls
    // We got the lock read MPI messages if any.
    std::lock_guard<std::mutex> lock(mpiMutex, std::adopt_lock);
    return receiveOneEvent();
}

Event*
MultiThreadedShmCommunicator::receiveOneEvent() {
    // Now proceed with MPI operations
    MPI_STATUS status;
    try {
        if (!MPI_IPROBE(MPI_ANY_SOURCE, MPI_ANY_TAG, status)) {
            // No pending event.
            return NULL;
        }
    } catch (CONST_EXP MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        return NULL;
    }
    // Figure out the agent list size
    int eventSize = MPI_GET_COUNT(status, MPI_TYPE_CHAR);
    // Note we pass -1 to allocate event on this_thread's NUMA node.
    char *incoming_event = Event::allocate(eventSize, -1);
    ASSERT( incoming_event != NULL );
    // Read the actual data.
    try {
        MPI_RECV(incoming_event, eventSize, MPI_TYPE_CHAR,
                 status.MPI_SOURCE, status.MPI_TAG, status);
    } catch (CONST_EXP MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        delete[] incoming_event;
        return NULL;
    }    
    // The logic of how incoming messages are handled here is
    // different than the base class implementation.
    ASSERT((status.MPI_TAG == EVENT) || (status.MPI_TAG == GVT_MESSAGE));
    // Type cast does the trick as events are binary blobs
    Event* the_event = reinterpret_cast<Event*>(incoming_event);
    // Since the event is from the network, the reference count must be 1
    ASSERT(the_event->getReferenceCount() == 1);
    // Rest of the logic associated with GVT manager inspecting events
    // etc. is done in the MultiThreadedShmSimulation's
    // processIncomingEvents() method instead of here.
    return the_event;
}

int
MultiThreadedShmCommunicator::receiveManyEvents(EventContainer& eventList,
                                             const int maxEvents,
                                             int retryCount) {
    // First try to get the lock on the MPI mutex to ensure
    // thread-safe operations.
    while (retryCount > 0) {
        if (mpiMutex.try_lock()) {
            break;  // yay! got exclusive lock.
        }
        retryCount--;  // counter to prevent too many trials.
    }
    if (retryCount <= 0) {
        // Could not get mpi mutex. Can't read messages.
        return 0;
    }
    // When control drops here we have thread-safe access to MPI
    std::lock_guard<std::mutex> lock(mpiMutex, std::adopt_lock);
    int eventCount = 0;
    while (eventCount < maxEvents) {
        muse::Event* const event = receiveOneEvent();
        if (event != NULL) {
            eventList.push_back(event);  // Add received events
            eventCount++;                // Track events read.
        } else {
            // No pending events. time to get out.
            break;
        }
    }
    // Return the number of events read
    return eventCount;
}

#endif
