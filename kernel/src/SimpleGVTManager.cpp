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
// Authors: Jingbin Yu       yuj53@miamioh.edu
//
//---------------------------------------------------------------------------
#include "SimpleGVTManager.h"
#include "DataTypes.h"
#include "Communicator.h"
#include "Event.h"
#include "EventAdapter.h"
#include "Simulation.h"

using namespace muse;

muse::SimpleGVTManager::SimpleGVTManager(Simulation *sim): GVTManagerBase(sim) {

}

void muse::SimpleGVTManager::initialize(const Time &startTime, Communicator *comm) {
    ASSERT(startTime < TIME_INFINITY);
    ASSERT(comm != nullptr);

    gvt         = startTime;
    commManager = comm;

    unsigned int numThreads;
    commManager->getProcessInfo(rank, numProcesses, numThreads);

    ASSERT(numProcesses > 0);
    ASSERT(rank < numProcesses);
}

bool muse::SimpleGVTManager::sendRemoteEvent(Event *event) {
    // We don't do anything else but just send the event.
    // This method exists in the base class for GVTManager
    commManager->sendEvent(event, EventAdapter::getEventSize(event));
    return true;
}

inline Time muse::SimpleGVTManager::getGVT() {
    return gvt;
}

void muse::SimpleGVTManager::forceUpdateGVT() {
    ASSERT(commManager != nullptr);
    DEBUG(std::cout << "Simulator with rank " << sim->myID
                    << " invoked forceUpdateGVT()" << std::endl);

    // If there is only 1 process, we just use its LGVT
    // as GVT
    if (sim->getNumberOfProcesses() < 2) {
        gvt = sim->getLGVT();
        return;
    }

    // Call MPI_AllReduce on current process
    // and update the GVT value
    allReduceLGVTAndUpdateGVT();
}

void muse::SimpleGVTManager::allReduceLGVTAndUpdateGVT() {
    ASSERT(sim != nullptr);

    Time LGVT2Send = sim->getLGVT(), GVTUpdated;
    
    MPI_ALL_REDUCE(&LGVT2Send, &GVTUpdated, 1, MPI_DOUBLE, MPI_MIN);

    ASSERT(GVTUpdated >= gvt && "New GVT should not be smaller than the previous GVT");

    DEBUG(std::cout << "Simulation with rank " << sim->myID
                    << " sent out LGVT: " << LGVT2Send
                    << " and received Updated GVT: " << GVTUpdated
                    << std::endl);

    // After we get the updated GVT, we update the internal gvt variable
    gvt = GVTUpdated;
}

void muse::SimpleGVTManager::inspectRemoteEvent(Event *event) {
    UNUSED_PARAM(event);
}