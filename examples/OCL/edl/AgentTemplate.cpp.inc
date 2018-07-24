R"(
#ifndef %CLASS_NAME%_AGENT_CPP
#define %CLASS_NAME%_AGENT_CPP

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

#include "%CLASS_NAME%Agent.h"
#include "OclEqnSolvers.h"

// Definitions for the static variables
int %CLASS_NAME%Agent::rows = 0;
int %CLASS_NAME%Agent::cols = 0;

void
%CLASS_NAME%Agent::initialize() throw (std::exception) {
    // Schedule an event to ourselves at the next time step.
    scheduleEvent(muse::Event::create<muse::Event>(getAgentID(),
                                                   getTime() + 1));
    // Let the base class know we want to run a custom HC kernel
    runHCkernel(0);
}

void
%CLASS_NAME%Agent::executeTask(const muse::EventContainer& events) {
    UNUSED_PARAM(events);
    %CLASS_NAME%State *state = dynamic_cast<%CLASS_NAME%State*>(getState());
    ASSERT( state != NULL );
    oss << "Agent #" << getAgentID() << ", " << getTime();
    for (int i = 0; (i < COMPARTMENTS); i++) {
        oss << ", " << state->hc.comps[i];
    }
    oss << std::endl;    
    scheduleEvent(muse::Event::create<muse::Event>(getAgentID(),
                                                   getTime() + 1));
    // Let the base class know we want to run a custom HC kernel
    runHCkernel(0);    
}

void
%CLASS_NAME%Agent::finalize() {
}

void
%CLASS_NAME%Agent::setParams(int rows, int cols) {
    %CLASS_NAME%Agent::rows = rows;
    %CLASS_NAME%Agent::cols = cols;
}

std::string
%CLASS_NAME%Agent::getHCPreSupportCode() {
    std::string define = "#define COMPARTMENTS " +
        std::to_string(COMPARTMENTS) + "\n\n" +
        "#define PARAMETERS " + std::to_string(PARAMETERS) + "\n\n";
    return define + EquationSolvers;
}

HC_POST_SOURCE(%CLASS_NAME%Agent, %CLASS_NAME%State,
               %AGENT_HC_SOURCE%
);

%CLASS_NAME%Agent::%CLASS_NAME%Agent(muse::AgentID id, int x, int y,
                                 bool ode, real step,
                                     const std::vector<real>& compVals) :
    muse::HCAgent(id, new %CLASS_NAME%State()), x(x), y(y) {
    // Setup the initial populations in the state
    %CLASS_NAME%State *state = dynamic_cast<%CLASS_NAME%State*>(getState());
    for (size_t i = 0; (i < compVals.size()); i++) {
        state->hc.comps[i] = compVals[i];
    }
    // Setup default values for epidemic parameters
    hc.ode       = ode;
    hc.step      = step;
    // Setup values in hc.params here as needed.
}

// Define the HC kernel code using MUSE's macros
HC_KERNEL(%CLASS_NAME%Agent, %CLASS_NAME%State,
          UNUSED_PARAM(id);
          UNUSED_PARAM(rndInfo);
          UNUSED_PARAM(lvt);
          UNUSED_PARAM(gvt);
          real rates[8];
          if (!hc_params->ode) {
              struct MTrand_Info mt = *rndInfo;
              SSA_gillespieTauLeap(hc_params->step, 8, rates,
                            %CLASS_NAME%_EventChange, hc_params->params,
                            COMPARTMENTS, hc_state->comps,
                            &mt);
              *rndInfo = mt;  // Copy rnd generator status back
          } else {
              RK_solver(hc_params->step, COMPARTMENTS, hc_params->params,
                        hc_state->comps, hc_state->comps);
          }
);

// Load the helper equation solvers as a C++11 raw string to be used
// to create an OpenCL kernel.
const std::string %CLASS_NAME%Agent::EquationSolvers =
#include "OclEqnSolvers.c_inc"
;

#endif
)"
