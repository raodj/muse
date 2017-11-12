#ifndef GVT_MANAGER_CPP
#define	GVT_MANAGER_CPP

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
//          Meseret R. Gebre       meseret.gebre@gmail.com
//
//---------------------------------------------------------------------------

#include "GVTManager.h"
#include "GVTMessage.h"
#include "Communicator.h"
#include "Simulation.h"
#include "Event.h"

#include <cstring>

// switch to muse namespace to make life easier.
using namespace muse;

GVTManager::GVTManager(muse::Simulation* sim) : sim(sim) {
    ASSERT( sim != NULL );
    // Initialize all the instance variables to default values.
    white          = 0;    // White is 0 to begin with
    vecCounters[0] = NULL; // Vectors are created in initialize
    vecCounters[1] = NULL;
    // Set time to some invalid values.
    tMin           = TIME_INFINITY;
    gvt            = TIME_INFINITY;
    numProcesses   = 0;
    pendingAcks    = 0;
    cycle          = 0;
    commManager    = NULL;
    ctrlMsg        = NULL;
    rank           = -1u;
    // Set active color to white
    activeColor    = white;
}

GVTManager::~GVTManager() {
    if (vecCounters[0] != NULL) {
        // Free dynamically allocated memory
        delete[] vecCounters[0];
        delete[] vecCounters[1];
    }
}

void
GVTManager::initialize(const Time& startTime, Communicator* comm) {
    // Validate parameters
    ASSERT(comm != NULL);
    ASSERT(startTime < TIME_INFINITY);
    
    // Re-initialize the instance variables using new info.
    gvt          = startTime;
    commManager  = comm;
    // Determine configuration information from the comm manager.
    unsigned int numProcs;  // Dummy. Not used.
    commManager->getProcessInfo(rank, numProcs, numProcesses);
    ASSERT(numProcesses > 0);
    ASSERT(rank < numProcesses);
    
    // Create the main vector counters.
    vecCounters[0] = new int[numProcesses];
    vecCounters[1] = new int[numProcesses];
    // zero out all vector counters.
    memset(vecCounters[0], 0, sizeof(int) * numProcesses);
    memset(vecCounters[1], 0, sizeof(int) * numProcesses);
}

bool
GVTManager::sendRemoteEvent(Event *event) {
    // Validate conditions.
    ASSERT(event != NULL);
    ASSERT(commManager != NULL);
    
    // Compute remote process id.
    const unsigned int destRank =
        commManager->getOwnerThreadRank(event->getReceiverAgentID());
    ASSERT(destRank < numProcesses);

    // Perform the operations related to sending of a message as
    // described in Mattern's paper. First set color of event and ship
    // it out.
    ASSERT((activeColor == 0) || (activeColor == 1));
    event->setColor(activeColor);
    // Now track event counters immaterial of whether the event is
    // white or red in perparation for the next cycle where the values
    // of white and red will be swapped.
    vecCounters[(int) activeColor][destRank]++;
    // For non-white messages track minimum outgoing event time stamp
    // as well
    if (activeColor != white) {
        tMin = std::min<Time>(tMin, event->getReceiveTime());
        DEBUG(std::cout << "GVTManager tMin = " << tMin << std::endl);
    }
    // Finally actually ship out the event to the destination
    DEBUG(std::cout << *event << std::endl);
    // NOTE: Sending event should be last step.  After sending you
    // should not inspect the event because the receiving thread may
    // delete it immediately and inspecting event after that will
    // cause memory errors!
    commManager->sendEvent(event, event->getEventSize());
    // Everything went well.
    return true;
}

void
GVTManager::inspectRemoteEvent(Event *event) {
    ASSERT(event != NULL); // Ensure we have a valid event.
    // Event's color can be only 2 values -- white or !white
    ASSERT((event->getColor() == white) || (event->getColor() == !white));
    DEBUG(std::cout << "GVTManager inspected " << *event << std::endl);
    // Update vector counters associated with this process.
    vecCounters[(int) event->getColor()][rank]--;
    // Any waiting control message must be forwarded only after this
    // incoming event has been scheduled.
}

void
GVTManager::checkWaitingCtrlMsg() {
    if (ctrlMsg == NULL) {
        // No pending control msg. Nothing further to do.
        return;
    }
    ASSERT(ctrlMsg->getKind() == GVTMessage::GVT_CTRL_MSG);
    // There is a pending message. Check if wait expiration condition
    // has been met
    const int *count = ctrlMsg->getCounters();
    ASSERT(count != NULL);
    if (count[rank] + vecCounters[(int) white][rank] > 0) {
        // We still need to continue to wait.
        return;
    }
    ASSERT(count != NULL);
    // When control drops here that means the wait has expired and the
    // pending GVT control message must be updated and forwarded to
    // the next process in the loop if necessary.
    DEBUG(std::cout << "Checking waiting ctrlMsg: " << *ctrlMsg
                    << ", tMin = " << tMin << std::endl);
    if ((rank == ROOT_KERNEL) && (ctrlMsg->areCountersZero(numProcesses))) {
        ASSERT (tMin >= ctrlMsg->getMin());
        // A phase of GVT computation is done.
        setGVT(ctrlMsg->getMin());
        // Get rid of the control message as we no longer need it.
        GVTMessage::destroy(ctrlMsg);
        ctrlMsg = NULL;
    } else {
        // Forward control message to the next process
        forwardCtrlMsg();
    }
}

void
GVTManager::forwardCtrlMsg() {
    // First update the counters in the message and reset local
    // counters.
    ASSERT(ctrlMsg != NULL);
    int *count = ctrlMsg->getCounters();
    ASSERT(count != NULL);
    // Update all the vector counters
    for(unsigned int pid = 0; (pid < numProcesses); pid++) {
        count[pid] += vecCounters[(int) white][pid];
        vecCounters[(int) white][pid] = 0;
    }
    // Update tmin.
    ctrlMsg->setTmin(std::min<Time>(ctrlMsg->getTmin(), tMin));
    // Set GVT estimate based on rank of process. But first determine
    // our LGVT value.
    const Time lgvt = sim->getLGVT();
    DEBUG(std::cout << "Process " << rank << " LGVT: " << lgvt
                    << ", tMin = " << ctrlMsg->getTmin() << std::endl);
    if (rank != ROOT_KERNEL) {
        // This is non-initiator sequence.
        ctrlMsg->setGVTEstimate(std::min<Time>(ctrlMsg->getGVTEstimate(),lgvt));
    } else {
        // This is initiator sequence. Note that lgvt (aka m_clock
        // from Mattern's paper) is not accumulated across rounds.
        ctrlMsg->setGVTEstimate(lgvt);
    }
    // Now send control message to next process
    const int dest = (rank + 1) % numProcesses;
    ctrlMsg->setDestRank(-dest);  // Facilitate multi-threaded routing
    commManager->sendMessage(ctrlMsg, dest);
    DEBUG(std::cout << "Sent from " << rank << " to " << dest
                    << " GVT ctrlMsg: " << *ctrlMsg << std::endl);
    // We no longer have a control message.
    GVTMessage::destroy(ctrlMsg);
    ctrlMsg = NULL;
    // Update cycle counters for GVT token circulation
    cycle++;
    ASSERT(cycle < 3);
}

void
GVTManager::recvGVTMessage(GVTMessage *message) {
    ASSERT(message != NULL);
    ASSERT(message->getKind() != GVTMessage::INVALID_GVT_MSG);
    
    if (message->getKind() == GVTMessage::GVT_EST_MSG) {
        ASSERT(rank != 0);
        ASSERT(ctrlMsg == NULL);
        ASSERT(activeColor != white);
        // We have a new gvt estimate. update and garbage collect.
        setGVT(message->getGVTEstimate());
        DEBUG(std::cout << "Rank " << rank << " set GVT via: "
                        << *message << std::endl);
        // Delete this message as we no longer need it.
        GVTMessage::destroy(message);
        // All done with this case.
        return;
    } else if (message->getKind() == GVTMessage::GVT_ACK_MSG) {
        // Acknowledgement for gvt update from a remote process
        ASSERT(rank == ROOT_KERNEL);
        ASSERT(pendingAcks > 0);
        pendingAcks--;
        if (pendingAcks == 0) {
            // All acks received. Finally update GVT on local rank
            garbageCollect(message->getGVTEstimate());
        }
        // Delete this message as we no longer need it.
        GVTMessage::destroy(message);
        return;
    }
    
    // When control drops here we are handing a control message.
    ASSERT(message->getKind() == GVTMessage::GVT_CTRL_MSG);
    ASSERT(ctrlMsg == NULL);
    // Setup the new control message.
    ctrlMsg = message;
    DEBUG(std::cout << "Recieved GVT ctrlMsg: " << *ctrlMsg << std::endl);
    // Change our current active color if needed.
    if ((rank != ROOT_KERNEL) && (activeColor == white)) {
        activeColor = !white;
        tMin        = TIME_INFINITY;
    }
    // Let the helper method do rest of the processing.
    checkWaitingCtrlMsg();
}

void
GVTManager::startGVTestimation() {
    if ((cycle != 0) || (rank != 0) || (pendingAcks != 0)) {
        // Either this is not the initator process or a GVT
        // calculation is already underway. Do nothing either way.
        return;
    }
    
    if (numProcesses < 2) {
        // If there is only one process in the simulation, then simply
        // use LGVT as the GVT value!
        activeColor = !white; // Change our active color.
        setGVT(sim->getLGVT());
        return;
    }
    
    // When control drops here, we are starting a new round of gvt
    // computation with 2 or more processes
    ASSERT(rank == ROOT_KERNEL);
    ASSERT(cycle == 0);
    ASSERT(ctrlMsg == NULL);
    ASSERT(activeColor == white);
    // Start a new GVT estimation process.
    GVTMessage *msg = GVTMessage::create(GVTMessage::GVT_CTRL_MSG,
                                         numProcesses, -1);
    ASSERT(msg != NULL);
    // Fill in other details.
    msg->setGVTEstimate(sim->getLGVT());
    msg->setTmin(TIME_INFINITY);
    // Update and reset the vector counters.
    int *count = msg->getCounters();
    ASSERT(count != NULL);
    for(unsigned int pid = 0; (pid < numProcesses); pid++) {
        count[pid] = vecCounters[(int) white][pid];
        vecCounters[(int) white][pid] = 0;
    }
    
    // Now toggle our state and update tmin.
    activeColor = !white;
    tMin        = TIME_INFINITY;
    // Track GVT cycle in process.
    cycle       = 1;
    // Dispatch the message to the next process.
    commManager->sendMessage(msg, 1);
    DEBUG(std::cout << "Starting GVT estimate: " << *msg << std::endl);
    // Delete message as we no longer need it.
    GVTMessage::destroy(msg);
}

void
GVTManager::setGVT(const Time& gvtEst) {
    // First swap our definition of red and white putting this process
    // in white state (it must be red right now)
    ASSERT(activeColor == !white);
    white = !white;  // Changing definition of white maybe a bit
                     // confusing but it is valid and correct.
    ASSERT(activeColor == white);
    // Reset cycle of GVT computation.
    cycle = 0;
    
    // Next update gvt if the new estimate is different
    ASSERT(gvtEst >= gvt);
    
    if (rank == ROOT_KERNEL) {
        // Have a new and useful GVT value. First broadcast it to all
        // other processes using a suitable GVT message.
        GVTMessage *gvtMsg = GVTMessage::create(GVTMessage::GVT_EST_MSG);
        ASSERT(gvtMsg != NULL);
        gvtMsg->setGVTEstimate(gvtEst);
        // Update the pending acks variable first.
        ASSERT(pendingAcks == 0);
        pendingAcks = numProcesses - 1;
        // Here is the broadcast loop.
        for(unsigned int pid = 1; (pid < numProcesses); pid++) {
            gvtMsg->setDestRank(-pid);
            commManager->sendMessage(gvtMsg, pid);
        }
        // Destroy message as we no longer need it.
        GVTMessage::destroy(gvtMsg);
        // If we don't have any pending acks, then trigger garbage collect
        if (pendingAcks == 0) {
            // No need to wait for acknowledgments (as we will get none)
            garbageCollect(gvtEst);
        }
    } else {
        // Received a GVT update from process 0. Send an
        // acknowledgement back.
        GVTMessage *gvtAck = GVTMessage::create(GVTMessage::GVT_ACK_MSG, 0, 0);
        ASSERT(gvtAck != NULL);
        gvtAck->setGVTEstimate(gvtEst);
        // Send ack to process 0
        commManager->sendMessage(gvtAck, ROOT_KERNEL);
        // Destroy message as we no longer need it.
        GVTMessage::destroy(gvtAck);
        // Trigger garbage collection right away on non-zero ranks
        garbageCollect(gvtEst);
    }
}

void
GVTManager::garbageCollect(const Time& gvtEst) {
    // Trigger garbage collection in the simulation if gvt has
    // actually changed
    if (gvtEst > gvt) {
        // Update our local GVT value.
        gvt = gvtEst;
        // Report GVT value only on root kernel
        if (rank == ROOT_KERNEL) {
            // Report GVT update
            std::cout << "GVT: " << gvtEst << std::endl;
        }
        DEBUG(std::cout << "GVT: " << gvtEst << std::endl);
        // Do garbage collection
        ASSERT( sim != NULL );
        sim->garbageCollect();
    }
}

#endif
