#ifndef MUSE_EXAMPLES_OCL_EBOLAAGENT_H_
#define MUSE_EXAMPLES_OCL_EBOLAAGENT_H_
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

#include "OclAgent.h"
#include "OclState.h"
#include <string>

typedef float real;
class EbolaAgent : public muse::OclAgent {
    friend class OclSimulation;

    public:
        int compartments;
        std::string country;
        real inf;
        EbolaAgent(muse::AgentID id, muse::OclState* state, std::string country);

        /*
         * Helper function for nextODE
         * function runs seir equations with values
         * specific to the disease being modeled.
         */
         void seir(const real* xl, real* xln);

        /*
         Makes a step with the ODE equations -
         * runs Runge Kutta fourth order equations
         * to advance the agent one time step in the simulation
         * 
         * xl is the values of the current state and they are updated
         * within the method
         */
         void nextODE(real* xl);

        /*
         Makes a step with the SSA equations -
         * runs Gillespie with Tau Leaping optimization
         * to advance the agent one time step in the simulation
         * 
         * cv is the values of the current state and they are updated
         * within the method
         */
         void nextSSA(real* cv);

         /*
          * Returns the kernel code for this type of agent
          * Called from the oclSimulation class
          * Allows for multiple agent types
          */
         std::string getKernel();
};


#endif  // MUSE_EXAMPLES_OCL_EBOLAAGENT_H_
