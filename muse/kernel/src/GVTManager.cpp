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
// Authors:  Dhananjai M. Rao       raodm@muohio.edu
//           
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

GVTManager::GVTManager() {
    // Initialize all the instance variables to default values.
    white          = 0;    // White is 0 to begin with
    vecCounters[0] = NULL; // Vectors are created in initialize
    vecCounters[1] = NULL;
    // Set time to some invalid values.
    tMin           = INFINITY;
    gvt            = INFINITY;
    numProcesses   = 0;
    cycle          = 0;
    commManager    = NULL;
    ctrlMsg        = NULL;
    // Set active color to white
    activeColor    = white;
    if (rank == 0)cout << "In GVTManager cTor tMin (" <<tMin <<") gvt (" <<gvt<<")" <<endl; 
}

GVTManager::~GVTManager() {
    cout << "GVTManager Destroyed" <<endl;
    if (vecCounters[0] != NULL) {
        // Free dynamically allocated memory
        delete[] vecCounters[0];
        delete[] vecCounters[1];
    }
}

void
GVTManager::initialize(const Time& startTime, Communicator *comm) {
    // Validate parameters
    ASSERT ( comm != NULL );
    ASSERT ( startTime < INFINITY );
    
    // Re-initialize the instance variables.
    gvt          = startTime;
    commManager  = comm;
    // Determine configuration information from the comm manager.
    commManager->getProcessInfo(rank, numProcesses);
    ASSERT ( numProcesses > 0 );
    ASSERT ( rank < numProcesses );
    
    // Create the main vector counters.
    vecCounters[0] = new unsigned int[numProcesses];
    vecCounters[1] = new unsigned int[numProcesses];
    // zero out all vector counters.
    memset(vecCounters[0], 0, sizeof(unsigned int) * numProcesses);
    memset(vecCounters[1], 0, sizeof(unsigned int) * numProcesses);
    if (rank == 0)cout << "In GVTManager initialize tMin (" <<tMin <<") gvt (" <<gvt<<")" <<endl; 
}

bool
GVTManager::sendRemoteEvent(Event *event) {
    // Validate conditions.
    ASSERT ( event != NULL );
    ASSERT ( commManager != NULL );

    // Compute remote process id.
    const unsigned int destRank =
        commManager->getOwnerRank(event->getReceiverAgentID());
    ASSERT ( destRank != rank );
    ASSERT ( destRank < numProcesses );
    
    // Perform the operations related to sending of a message as
    // described in Mattern's paper. First set color of event and ship
    // it out.
    ASSERT ((activeColor == 0) || (activeColor == 1));
    event->setColor(activeColor);
    commManager->sendEvent(event, event->getEventSize());
    // Now track event counters immaterial of whether the event is
    // white or red in perparation for the next cycle where the values
    // of white and red will be swapped.
    vecCounters[(int) activeColor][destRank]++;
    // For non-white messages track minimum outsoing event time stamp
    // as well
    if (activeColor != white) {
        tMin = std::min<Time>(tMin, event->getReceiveTime());
    }
    if (rank == 0)cout << "In GVTManager sendRemoteEvent tMin (" <<tMin <<") gvt (" <<gvt<<")" <<endl; 
    // Everything went well.
    return true;
}

void
GVTManager::inspectRemoteEvent(Event *event) {
    ASSERT ( event != NULL );
    // Update vector counters associated with this process.
    vecCounters[(int) event->getColor()][rank]--;
    // If there is a pending control message then update and forward
    // it as necessary.
    checkWaitingCtrlMsg();
}

void
GVTManager::checkWaitingCtrlMsg() {
    if (ctrlMsg == NULL) {
        // No pending control msg. Nothing further to do.
        return;
    }
    ASSERT ( ctrlMsg->getKind() == GVTMessage::GVT_CTRL_MSG );
    // There is a pending message. Check if wait expiration
    // condition has been met
    const unsigned int *count = ctrlMsg->getCounters();
    ASSERT ( count != NULL );
    if (count[rank] + vecCounters[(int) white][rank] > 0) {
        // We still need to continue to wait.
        return;
    }
    // When control drops here that means the wait has expired and the
    // pending GVT control message must be updated and forwarded to
    // the next process in the loop if necessary.
    if ((rank == ROOT_KERNEL) && (ctrlMsg->areCountersZero(numProcesses))) {
        // A phase of GVT computation is done.
        setGVT(std::min<Time>(ctrlMsg->getGVTEstimate(), ctrlMsg->getTmin()));
        if (rank == 0)cout << "(GVT phase ended) In GVTManager checkWaitingCtrlMsg  ctrlMsg->tMin (" <<
                          ctrlMsg->getTmin() <<") ctrlMsg->gvtEst (" << ctrlMsg->getGVTEstimate() <<")" <<endl;
        // Get rid of the control message as we no longer need it.
        GVTMessage::destroy(ctrlMsg);
        ctrlMsg = NULL;
        if (rank == 0)cout << "(GVT phase ended) In GVTManager checkWaitingCtrlMsg  tMin (" <<
                          tMin <<") gvt (" <<gvt<<")" <<endl;
        
    } else {
        // Forward control message to the next process
        if (rank == 0)cout << "(GVT phase cont..) In GVTManager checkWaitingCtrlMsg  tMin (" <<
                          tMin <<") gvt (" <<gvt<<")" <<endl; 
        forwardCtrlMsg();
    }
}

void
GVTManager::forwardCtrlMsg() {
    // First update the counters in the message and reset local
    // counters.
    ASSERT ( ctrlMsg != NULL );
    unsigned int *count = ctrlMsg->getCounters();
    ASSERT ( count != NULL );
    for(unsigned int pid = 0; (pid < numProcesses); pid++) {
        count[pid] += vecCounters[(int) white][pid];
        vecCounters[(int) white][pid] = 0;
    }
    // Update tmin.
    //cout << " in forwardCtrlMsg tMIn is " << tMin <<endl;
    //cout << " in forwardCtrlMsg ctrlMsg->tMIn is " << ctrlMsg->getTmin() <<endl;
    if (rank == 0)cout << "In GVTManager forwardCtrlMsg tMin (" <<tMin <<") gvt (" <<gvt<<")" <<endl; 
    ctrlMsg->setTmin(std::min<Time>(ctrlMsg->getTmin(), tMin));
    // Set GVT estimate based on rank of process. But first determine
    // our LGVT value.
    const Time lgvt = Simulation::getSimulator()->getLGVT();
    if (rank != ROOT_KERNEL) {
        // This is non-initiator sequence.
        ctrlMsg->setGVTEstimate(std::min<Time>(ctrlMsg->getGVTEstimate(),lgvt));
    } else {
        ctrlMsg->setGVTEstimate(lgvt);
    }
    // Now send control message to next process
    commManager->sendMessage(ctrlMsg, (rank + 1) % numProcesses);
    // We no longer have a control message.
    GVTMessage::destroy(ctrlMsg);
    ctrlMsg = NULL;
    // Update cycle counters for GVT token circulation
    cycle++;
    ASSERT ( cycle < 3 );
}

void
GVTManager::recvGVTMessage(GVTMessage *message) {
    ASSERT ( message != NULL );
    if (message->getKind() == GVTMessage::GVT_EST_MSG) {
        ASSERT ( rank != 0 );
        // We have a new gvt estimate. update and garbage collect.
        setGVT(message->getGVTEstimate());
        // Delete this message as we no longer need it.
        GVTMessage::destroy(message);
        // All done with this case.
        return;
    }
    // When control drops here we are handing a control message.
    ASSERT ( message->getKind() == GVTMessage::GVT_CTRL_MSG );
    ASSERT ( ctrlMsg == NULL );
    // Setup the new control message.
    ctrlMsg = message;
    // Change our current active color if needed.
    if ((rank != ROOT_KERNEL) && (activeColor == white)) {
        activeColor = !white;
        tMin        = INFINITY;
    }
    // Let the helper method do rest of the processing.
    checkWaitingCtrlMsg();
}

void
GVTManager::startGVTestimation() {
    if ((cycle != 0) || (rank != 0)) {
        // Either this is not the initator process or a GVT
        // calculation is already underway. Do nothing either way.
        cout << "In GVTManager startGVTestimation BUT RETURNING: cycle ("<<cycle<<")"
             << "rank ("<<rank<<")" <<endl;
        return;
    }
    if (rank == 0)cout << "In GVTManager startGVTestimation tMin (" <<tMin <<") gvt (" <<gvt<<")" <<endl; 
    if (numProcesses < 2) {
        // If there is only one process in the simulation, then simply
        // use LGVT as the GVT value!
        activeColor = !white; // Change our active color.
        setGVT(Simulation::getSimulator()->getLGVT());
        return;
    }

    // When control drops here, we are starting a new round of gvt
    // computation with 2 or more processes
    ASSERT ( rank == ROOT_KERNEL );
    ASSERT ( cycle == 0 );
    ASSERT ( ctrlMsg == NULL );
    ASSERT ( activeColor == white );
    // Start a new GVT estimation process.
    GVTMessage *msg = GVTMessage::create(GVTMessage::GVT_CTRL_MSG,numProcesses);
    ASSERT ( msg != NULL );
    // Fill in other details.
    msg->setGVTEstimate(Simulation::getSimulator()->getLGVT());
    msg->setTmin(INFINITY);
    // Update and reset the vector counters.
    unsigned int *count = msg->getCounters();
    ASSERT ( count != NULL );
    for(unsigned int pid = 0; (pid < numProcesses); pid++) {
        count[pid] += vecCounters[(int) white][pid];
        vecCounters[(int) white][pid] = 0;
    }
    // Now toggle our state and update tmin.
    activeColor = !white;
    tMin        = INFINITY;
    // Track GVT cycle in process.
    cycle       = 1;
    // Dispatch the message to the next process.
    commManager->sendMessage(msg, 1);
    // Delete message as we no longer need it.
    GVTMessage::destroy(msg);
}

void
GVTManager::setGVT(const Time& gvtEst) {
    // First swap our definition of red and white putting this process
    // in white state (it must be red right now)
    ASSERT ( activeColor == !white );
    white = !white;
    ASSERT ( activeColor == white );
    // Reset cycle of GVT computation.
    cycle = 0;
    
    // Next update gvt if the new estimate is different
    ASSERT ( gvtEst >= gvt );
    if (gvtEst <= gvt) {
        // Nothing special to be done in this case.
        return;
    }

    if (rank == ROOT_KERNEL) {
        // Have a new and useful GVT value. First broadcast it to all
        // other processes using a suitable GVT message.
        GVTMessage *gvt = GVTMessage::create(GVTMessage::GVT_EST_MSG);
        ASSERT ( gvt != NULL );
        gvt->setGVTEstimate(gvtEst);
        // Here is the broadcast loop.
        for(unsigned int pid = 1; (pid < numProcesses); pid++) {
            commManager->sendMessage(gvt, pid);
        }
        // Destroy message as we no longer need it.
        GVTMessage::destroy(gvt);
    }
    
    // Update our local GVT value.
    gvt = gvtEst;
    // std::cout << "GVT (rank: " << rank << ") set GVT to "
    //         << gvt << " with tMin set to " << tMin <<std::endl;
    // Trigger garbage collection in the simulation
    
    Simulation::getSimulator()->garbageCollect(gvt);
      
}

#endif
