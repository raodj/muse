#ifndef OCL_SYNTH_AGENT_CPP
#define OCL_SYNTH_AGENT_CPP

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

#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include "SynthAgent.h"
#include "OclEqnSolvers.h"

// Definitions for some of the  static variables
int  SynthAgent::rows           = 0;
int  SynthAgent::cols           = 0;
int  SynthAgent::transitions    = 0;
bool SynthAgent::showEpiVals    = false;
int  SynthAgent::eventsPerAgent = 3;
int  SynthAgent::granularity    = 10;  // 10 microseconds per event

// Static (shared between all the synthentic agents) definition for
// the SSA compartment changes
std::vector<real> SynthAgent::compChanges;

SynthAgent::SynthAgent(muse::AgentID id, int x, int y,
                       int solver, real step, int compartments,
                       int paramCount) :
    muse::HCAgent(id, new SynthAgentState(compartments)), x(x), y(y),
    solverType(solver), step(step), compartments(compartments),
    paramCount(paramCount), params(paramCount) {
    // Setup the initial populations in the state
    SynthAgentState *state = dynamic_cast<SynthAgentState*>(getState());
    ASSERT(state != NULL);
    state->comps[0] = 1000;
    state->comps[1] = 1;
    state->comps[2] = 2;
    // Setup default values for parameters.
    for (int i = 0; (i < paramCount); i++) {
        params[i] = i + id;
    }
}

void
SynthAgent::initialize() {
    // Schedule an event to ourselves at the next time step.
    for (int i = 0; (i < eventsPerAgent); i++) {
        scheduleEvent(muse::Event::create<muse::Event>(getAgentID(),
                                                       getTime() + 1));
    }
    // Let the base class know we want to run a custom HC kernel
    runHCkernel(0);
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
double
SynthAgent::simGranularity(int granularity) const {
    double sum = 0;
    for (int i = 0; (i < granularity); i++) {
        // The 50L is a magic number for owens
        for (size_t delay = 0; (delay < 45L); delay++) {
            sum += sin(0.5);
        }
    }
    return sum;
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize ("O0")
void
SynthAgent::executeTask(const muse::EventContainer& events) {
    UNUSED_PARAM(events);
    // For each event simulate some CPU load
    double sum = 0;
    for (size_t i = 0; (i < events.size()); i++) {
        sum += simGranularity(granularity);
        // Schedule new event for ourselves at next time step.
        scheduleEvent(muse::Event::create<muse::Event>(getAgentID(),
                                                       getTime() + 1));
    }
    // Let the base class know we want to run a custom HC kernel
    runHCkernel(0);
}
#pragma GCC pop_options

void
SynthAgent::finalize() {
    if (showEpiVals) {
        SynthAgentState *state = dynamic_cast<SynthAgentState*>(getState());
        ASSERT( state != NULL );
        oss << "Agent # " << getAgentID() << ":";
        for (int i = 0; (i < compartments); i++) {
            oss << " " << state->comps[i];
        }
        oss << std::endl;
    }
}

void
SynthAgent::setParams(int rows, int cols, int transitions,
                      int compartments, bool showEpiVals,
                      int eventsPerAgent, int granularity) {
    SynthAgent::rows           = rows;
    SynthAgent::cols           = cols;
    SynthAgent::transitions    = transitions;
    SynthAgent::showEpiVals    = showEpiVals;
    SynthAgent::eventsPerAgent = eventsPerAgent;
    SynthAgent::granularity    = granularity;
    // Generate the synthetic transition matrix
    generateSynthTransitions(transitions, compartments);
}

void
SynthAgent::generateSynthTransitions(const int transitions,
                                     const int compartments) {
    // use current time as seed for random generator rand() used below.
    std::srand(std::time(nullptr)); 
    // Clear out any existing transitions
    compChanges.clear();
    // Create "compartment" number of entries for each transition
    for (int tr = 0; (tr < transitions); tr++) {
        for (int comp = 0; (comp < compartments); comp++) {
            // Create +1, 0, or -1 transition with probability 20%,
            // 60%, 20% respectively.
            const int rnd  = std::rand() % 10;
            int delta = 1;  // change in compartment value.
            if (rnd < 2) {
                delta = -1;
            } else if (rnd < 8) {
                delta = 0;
            }
            // Add compartment change
            compChanges.push_back(delta);
        }
    }
    ASSERT((int) compChanges.size() == (transitions * compartments));
}

std::string
SynthAgent::getHCPreSupportCode() {
    std::ostringstream oclCode;
    oclCode << "typedef double real;\n\n"
            << "#define COMPARTMENTS " << compartments  << "\n\n"
            << "#define PARAMETERS "   << params.size() << "\n\n"
            << "#define TRANSITIONS "  << transitions   << "\n\n";
    // Now generate the compartment transition matrix used for SSA
    oclCode << "__constant real compChanges[] = {\n";
    for (size_t i = 0; (i < compChanges.size()); i++) {
        if (i % compartments == 0) {
            oclCode << "\n   ";  // New line for readability.
        }
        oclCode << compChanges[i] << ", ";
    }
    oclCode << "\n};\n\n";
    // Return the generated code as a string.
    return oclCode.str();
}

std::string
SynthAgent::getHCPostSupportCode() {
    return EquationSolvers;
}

std::string
SynthAgent::getHCkernelDefinition() {
    return SynthKernel;
}
       

std::string
SynthAgent::getHCParamDefinition() const {
    std::ostringstream os;
    os << "struct hc_params {\n"
       << "  int solverType;\n"
       << "  real step;\n"
       << "  real params[" << params.size() << "];\n"
       << "};\n\n";
    return os.str();
}

void
SynthAgent::copyToDevice(void *dest, const int bufferSize) {
    UNUSED_PARAM(bufferSize);
    // Setup the solverType
    int *solver = reinterpret_cast<int*>(dest);
    *solver     = solverType;
    // Copy parameters to the device.  Each value is 8-byte aligned.
    double *buff = reinterpret_cast<double*>(dest);
    buff[1]      = step;
    // Copy the parameter values
    for (size_t i = 0; (i < params.size()); i++) {
        buff[i + 2] = params[i];
    }
}

void
SynthAgent::executeHCkernel() {
    // This method performs the same operations as the
    // SyntAgentKernel.c
    SynthAgentState* const state = dynamic_cast<SynthAgentState*>(getState());
    ASSERT(state != NULL);
    // Call RK_solver, SSA, or SSA-Tau+Leap based on solverType
    switch (solverType) {
    case 0: {
        RK_solver(step, compartments, &params[0], &state->comps[0],
                  &state->comps[0]);
        break;
    }
    case 1: {
        std::vector<real> rates(transitions, 0);
        SSA_gillespie(step, transitions, &rates[0], &compChanges[0],
                      &params[0], compartments, &state->comps[0],
                      &rndInfo);
        break;
    }
    case 2: {
        std::vector<real> rates(transitions, 0);
        SSA_gillespieTauLeap(step, transitions, &rates[0], &compChanges[0],
                             &params[0], compartments, &state->comps[0],
                             &rndInfo);
        break;
    }
    default:
        std::cerr << "Invalid solver type encountered.\n";
        abort();
    }
}

// Load the helper equation solvers as a C++11 raw string to be used
// to create an OpenCL kernel.
const std::string SynthAgent::EquationSolvers =
#include "OclEqnSolvers.c_inc"
;

// Load the helper equation solvers as a C++11 raw string to be used
// to create an OpenCL kernel.
const std::string SynthAgent::SynthKernel =
#include "SynthKernel.c_inc"
;

#endif
