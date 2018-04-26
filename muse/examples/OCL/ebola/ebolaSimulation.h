#ifndef MUSE_EXAMPLES_OCL_EBOLASIMULATION_H
#define MUSE_EXAMPLES_OCL_EBOLASIMULATION_H
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
// Authors: Harrison Roth          rothhl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "oclSimulation.h"
#include "oclScheduler.h"
#include "ebolaAgent.h"
#include "oclState.h"
#include "AgentPQ.h"
#include <vector>

class ebolaSimulation {
    public:

        ebolaSimulation();

        /*
         * Create a set of agents and register them with the simulation 
         * This function uses simulation class variables to determine
         * specifics, like total number of agents.
         */
        void createAgents();

        ebolaSimulation(int& argc, char* argv[]);
        
        bool ode;
        bool oclAvailable;
        int compartments;
        int rows;
        int cols;
        int popSize;
        int expSize;
        float step;
        int stopTime;
        std::string country;
};

#endif /* MUSE_EXAMPLES_OCL_EBOLASIMULATION_H */

