#ifndef MUSE_HC_AGENT_CPP
#define MUSE_HC_AGENT_CPP

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
// Authors: Dhananjai M. Rao    raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include "HCAgent.h"
#include "Simulation.h"
#include "MuseOclLibrary.h"

// Currently this source file has only static instance variable
// defintions
struct MTrand_Info muse::HCAgent::rndInfo = {{0}, 0, 0, (int) time(NULL) };

void
muse::HCAgent::runHCkernel(const int kernelID) {
    if (!Simulation::getSimulator()->hasHCsupport()) {
        if (getTime() > 0) {
            executeHCkernel();
        }
    } else {
        muse::Agent::runHCkernel(kernelID);
    }
}

#endif
