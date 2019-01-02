#ifndef MUSE_MULTI_THREADED_COMMUNICATOR_CPP
#define MUSE_MULTI_THREADED_COMMUNICATOR_CPP

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

#include "mpi-mt/MultiThreadedCommunicator.h"
#include "mpi-mt/MultiThreadedSimulationManager.h"
#include "GVTManager.h"
#include "GVTMessage.h"
#include "DataTypes.h"
#include "Event.h"
#include "Agent.h"

using namespace muse;

MultiThreadedCommunicator::MultiThreadedCommunicator(MultiThreadedSimulationManager* simMgr,
                                                     const int thrPerNode)
    : simMgr(simMgr), threadsPerNode(thrPerNode), numMpiProcesses(0) {
    ASSERT( simMgr != NULL );
    ASSERT( threadsPerNode > 0 );
    // Nothing else to be done here for now
}

MultiThreadedCommunicator::~MultiThreadedCommunicator() {
    // Nothing else to be done here for now
}

SimulatorID
MultiThreadedCommunicator::initialize(int argc, char* argv[],
                                      bool initMPI) {
    // The base class does the necessary initialization
    SimulatorID simID = Communicator::initialize(argc, argv, initMPI);
    // Set the actual number of MPI processes in the simulation now.
    unsigned int rank = 0, totNumThreads = 0;
    getProcessInfo(rank, numMpiProcesses, totNumThreads);
    // Finally return the simulation ID from base class
    return simID;
}

// Method run only on MPI process with rank zero
void
MultiThreadedCommunicator::gatherBroadcastAgentInfo(VecUint& idThrList) {
    ASSERT(myMPIrank == ROOT_KERNEL);
    // First we collect id<->thread mapping from all other processes
    for (size_t proc = 1; (proc < numMpiProcesses); proc++) {
        MPI_STATUS status;  // information on incoming message
        // Probe for a message from another MPI process to figure out size.
        MPI_PROBE(MPI_ANY_SOURCE, AGENT_LIST, status);
        // Figure out the agent list size
        size_t listSize = MPI_GET_COUNT(status, MPI_TYPE_UNSIGNED);
        // Grow idThrList by the list size.
        const size_t currSize = idThrList.size();
        idThrList.resize(currSize + listSize);
        // Read the actual data from the MPI process.
        MPI_RECV(&idThrList[currSize], listSize, MPI_TYPE_UNSIGNED,
                 status.MPI_SOURCE, AGENT_LIST, status);
    }
    // Now we have the full mapping aggregated in idThrList. It is
    // time to broadcast it to all the processes.  First send size of
    // the full list so that other processes can allocate memory
    // needed to receive the data.
    unsigned int listSize = idThrList.size();
    ASSERT( listSize % 2 == 0 );
    MPI_BCAST(&listSize, 1, MPI_TYPE_UNSIGNED, ROOT_KERNEL);
    // Next broadcast the actual mapping
    MPI_BCAST(idThrList.data(), listSize, MPI_TYPE_UNSIGNED, ROOT_KERNEL);
    std::cout << "Registration of " << (listSize / 2) << " agents complete!\n";
}

// Method run only on MPI process other than rank zero
void
MultiThreadedCommunicator::sendGatherAgentInfo(VecUint& idThrList) {
    ASSERT(myMPIrank != ROOT_KERNEL);
    // First send local id<->thread mapping to process with Rank #0
    MPI_SEND(idThrList.data(), idThrList.size(), MPI_TYPE_UNSIGNED,
             ROOT_KERNEL, AGENT_LIST);
    // Next, get broadcasted full size of mapping from rank #0
    int listSize = 0;
    MPI_BCAST(&listSize, 1, MPI_TYPE_UNSIGNED, ROOT_KERNEL);
    ASSERT( listSize % 2 == 0 );  // must be even number
    // Allocate necessary space to receive the data
    idThrList.resize(listSize);
    // Receive the full list from the root kernel
    MPI_BCAST(idThrList.data(), listSize, MPI_TYPE_UNSIGNED, ROOT_KERNEL);    
}

void
MultiThreadedCommunicator::registerAllAgents() {
    // Build the list of pairs of values in the form <agent_id,
    // global_thread_id>.
    const int mpiRank = myMPIrank;
    VecUint idList(agentThreadMap.size() * 2);
    // The global thread ID of thread #0 on this process
    const int glblThrStartIdx = threadsPerNode * mpiRank;
    size_t idx = 0;  // index in idList
    for (AgentIDSimulatorIDMap::value_type& entry : agentThreadMap) {
        idList[idx++] = entry.first;   // agent ID
        idList[idx++] = (glblThrStartIdx + entry.second);  // thread ID
    }
    // We need to merge agent<->thread mapping from all MPI proceses
    if (numMpiProcesses > 1) {
        // We need to merge mappings. Rank #0 first receives mappings
        // from all processes, merges them, and then broadcasts the
        // merged/full list to all the processes.
        if (mpiRank == ROOT_KERNEL) {
            // Do operations for Rank #0 -- modifies idList
            gatherBroadcastAgentInfo(idList);
        } else {
            // Do operations for all other ranks
            sendGatherAgentInfo(idList);
        }
    }
    // Now idList has the full list of agent<->threadID
    // mapping. Update agentMap in base class with the information.
    ASSERT(idList.size() % 2 == 0);
    for (size_t i = 0; (i < idList.size()); i += 2) {
        agentMap[idList[i]] = idList[i + 1];
    }
}

void
MultiThreadedCommunicator::finalize(bool stopMPI) {
    // For now the base class does all the necessary work.
    Communicator::finalize(stopMPI);
}

void
MultiThreadedCommunicator::getProcessInfo(unsigned int& rank,
                                          unsigned int& numProcesses,
                                          unsigned int& totNumThreads) {
    // Get basic information from parent process
    Communicator::getProcessInfo(rank, numProcesses, totNumThreads);
    // Update the total number of threads
    totNumThreads = numProcesses * threadsPerNode;
}

void
MultiThreadedCommunicator::sendEvent(Event* e, const int eventSize){
    // Do some basic sanity checks.
    ASSERT( e != NULL );
    // First check to see if the reciever is on this process. If so,
    // directly insert the event into its incoming queue.    
    const int thrID = getThreadID(e->getReceiverAgentID());
    ASSERT( thrID != getThreadID(e->getSenderAgentID()) );
    if (thrID != -1) {
        // This is a local event. Insert it into the receiver agent's
        // incoming queue in a MT-safe manner.
        simMgr->addIncomingEvent(thrID, e);
    } else {
        // This is a remote event. Let the base class dispatch it over
        // MPI in a MT-safe operations
        std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
        Communicator::sendEvent(e, eventSize);
    }
}

void
MultiThreadedCommunicator::sendMessage(const GVTMessage *msg,
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
        simMgr->addIncomingEvent(destRank % threadsPerNode, copy);
    } else {
        // Msg needs to be sent to a remote thread.
        // Let the base class dispatch it over MPI in a MT-safe
        // operations.
        std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
        Communicator::sendMessage(msg, rank);
    }
}

void
MultiThreadedCommunicator::sendMessage(const std::string& str,
                                       const int destRank, int tag) {
    // Ensure MT-safe MPI calls
    std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
    // Let base class do the actual operation.
    Communicator::sendMessage(str, destRank, tag);
}

std::string
MultiThreadedCommunicator::receiveMessage(int& recvRank, const int srcRank,
                                          int tag, bool blocking) {
    // Ensure MT-safe MPI calls
    std::lock_guard<std::mutex> lock(mpiMutex);  // Ensure MT-safe
    // Let base class do the actual operation.
    return Communicator::receiveMessage(recvRank, srcRank, tag, blocking);
}

Event*
MultiThreadedCommunicator::receiveEvent() {
    // Ensure MT-safe MPI calls
    // We got the lock read MPI messages if any.
    std::lock_guard<std::mutex> lock(mpiMutex, std::adopt_lock);
    return receiveOneEvent();
}

Event*
MultiThreadedCommunicator::receiveOneEvent() {
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
    // etc. is done in the MultiThreadedSimulation's
    // processIncomingEvents() method instead of here.
    return the_event;
}

int
MultiThreadedCommunicator::receiveManyEvents(EventContainer& eventList,
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
