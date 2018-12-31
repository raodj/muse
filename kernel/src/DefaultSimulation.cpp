#ifndef DEFAULT_SIMUALTION_CPP
#define DEFAULT_SIMUALTION_CPP

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

#include "DefaultSimulation.h"
#include "Communicator.h"

muse::DefaultSimulation::DefaultSimulation() {
    // Nothing to be done for now.
}

muse::DefaultSimulation::~DefaultSimulation() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

void
muse::DefaultSimulation::initialize(int& argc, char* argv[], bool initMPI) {
    commManager = new Communicator();
    myID = commManager->initialize(argc, argv, initMPI);
    unsigned int numThreads;  // dummy. not really used.
    commManager->getProcessInfo(myID, numberOfProcesses, numThreads);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);    
    // Finally, let the base-class perform generic initialization
    muse::Simulation::initialize(argc, argv, initMPI);
}

void
muse::DefaultSimulation::finalize(bool stopMPI, bool delCommMgr) {
    // Let base class do the necessary work (for now)
    Simulation::finalize(stopMPI, delCommMgr);
}

void
muse::DefaultSimulation::preStartInit() {
    // First let the base class do the necessary setup.
    muse::Simulation::preStartInit();
    // Next, we setup/finalize the AgentMap for all kernels
    commManager->registerAgents(allAgents);
}

#endif
