#ifndef SYNTH_KERNEL_C
#define SYNTH_KERNEL_C

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

#ifndef hc_state_type
typedef struct hc_state hc_state_type;
typedef struct hc_params hc_params_type;
#endif

/** A custom implementation of Heterogeneous Computing (HC) kernel
    function for synthethic simulation

    \param[in] id The ID of the agent for which this kernel is being
    launched.

    \param[in] hc_params The parameters associated with this agent.
    The parameter values are the same (at given lvt) as that on the
    CPU.

    \para[in] hc_state The state variables associated with this agent.
    This value is the same (at given lvt) as that on the CPU.
*/
void hcKernel(int id, const hc_params_type* hc_params,
              hc_state_type* hc_state,
              global struct MTrand_Info* rndInfo,
              const Time lvt, const Time gvt) {
    // Call RK_solver, SSA, or SSA-Tau+Leap based on solverType
    // printf("solverType = %d, step = %lf\n", hc_params->solverType,
    //        hc_params->step);
    switch (hc_params->solverType) {
    case 0:
        RK_solver(hc_params->step, COMPARTMENTS, hc_params->params,
                  hc_state->comps, hc_state->comps);
        break;

    case 1: {
        real rates[TRANSITIONS];
        struct MTrand_Info rndGenInfo = *rndInfo;
        SSA_gillespie(hc_params->step, TRANSITIONS, rates, compChanges,
                      hc_params->params, COMPARTMENTS, hc_state->comps,
                      &rndGenInfo);
        *rndInfo = rndGenInfo;
        break;
    }
        
    case 2: {
        real rates[TRANSITIONS];
        struct MTrand_Info rndGenInfo = *rndInfo;        
        SSA_gillespieTauLeap(hc_params->step, TRANSITIONS, rates, compChanges,
                             hc_params->params, COMPARTMENTS, hc_state->comps,
                             &rndGenInfo);
        *rndInfo = rndGenInfo;        
        break;
    }
    default:
        printf("Invalid solver type encountered.\n");
    }
}

#endif
