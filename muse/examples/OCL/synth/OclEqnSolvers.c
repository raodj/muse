#ifndef OCL_ODE_C
#define OCL_ODE_C

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
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

/** \file OclODE.c

    \brief This file contains an OpenCL compatible implementation for
    Equation solveers used for solving a system of ODE equations.

    \note This file is designed to be used both on the HOST and GPU to
    ensure operations are verly close (if not exactly the same) on
    both platforms.
*/

#ifndef real
typedef double real;
#endif

#ifndef UNUSED_PARAM
#define UNUSED_PARAM(x) (void) (x)
#endif

#ifndef __OPENCL_VERSION__
// Running on the host. We need definition for struct MTrand_Info
#include "MuseOclLibrary.h"
double log(double);
double sin(double);
#endif

/** Interface method to be implemented by application -- in this case
    the synthetic agent computes random changes in each compartment.

    This method is implemented here so that it can be readily used
    both on the CPU and GPGPU.  This method is associated with the
    synthetic agent which does not have "real world" ODEs.  Instead
    these are synthetic ODE's and the delta changes in them is
    computed using simple math reflecting what would get generated.
    We intentionally have a trigonometric function to include some
    compute load that would be present in realistic scenario.

    \param[in] numVars The number of variables/size of the
    currVals/deltas array.

    \param[in] params The parameters passed to RK_solve method.

    \param[in] currVals The current (intermediate) values.

    \param[out] deltas The resulting deltas in each variable to be
    computed by this method.
*/
void ode(const int numVars, const real params[],
         const real currVals[], real deltas[]) {
    UNUSED_PARAM(params);
    UNUSED_PARAM(currVals);
    // Generate some transitions.
    deltas[0] = 0;
    for (int i = 1; (i < numVars); i++) {
        deltas[0] += sin((real) i);
        deltas[i]  = (currVals[0] + currVals[i]) * 0.01 *
            ((i % 2 == 0 ? 1 : -1));
    }
    deltas[0] *= 0.01;
}

/** Interface method to be implemented by application -- in this case
    the synthetic agent computes random rates for each transition.

    This method is implemented here so that it can be readily used
    both on the CPU and GPGPU.  This method is associated with the
    synthetic agent which does not have "real world" rates.  Instead
    these are synthetic rates and the delta changes in them is
    computed using simple math.  We intentionally have a trigonometric
    function to include some compute load that would be present in
    realistic scenario.


    \param[in] numVars The number of variables/size of the
    currVals/deltas array.

    \param[in] params The parameters passed to RK_solve method.

    \param[in] currVals The current (intermediate) values.

    \param[out] deltas The resulting deltas in each variable to be
    computed by this method.
*/
void getSSARates(const int numRates, const real params[],
                 const int numComps, const real comps[], real rates[]) {
    UNUSED_PARAM(params);
    rates[0] = 0;
    for (int i = 1; (i < numRates); i++) {
        rates[0] += sin((real) i);
        rates[i] = (comps[i % numComps] < 100) ? 0.01 : -0.01;
    }
    rates[0] *= 0.01;
}

void RK_setInterim(const int numVars, const real currVals[],
                   const real k[], const real scale, real interim[]) {
    for (int i = 0; (i < numVars); i++) {
        interim[i] = currVals[i] + (k[i] * scale);
    }
}

void RK_copyArray(const int numVars, const real src[], real dest[]) {
    for (int i = 0; (i < numVars); i++) {
        dest[i] = src[i];
    }
}

/** The primary method for solving ODEs using Runge-Kutta 4th order
    method

    \param[in] numVars The size of the initValues and results array.

    \param[in] params This value is just passed through to ode()
    method.
    
    \param[in] initValues The initial set of values for various
    parameters in the DOE.

    \param[in] results The resulting values.

    \parma[in] step The step size to be used.
*/
void RK_solver(real step, const int numVars, const real params[],
               const real initValues[], real results[]) {
#ifndef __OPENCL_VERSION__
    const int COMPARTMENTS = numVars;
#endif
    // Initialize the running copy to initial value.
    real currVals[COMPARTMENTS];
    // Copy initial values
    RK_copyArray(numVars, initValues, currVals);
    // Use runge-kutta 4th order method here.
    for (real t = 0; (t < 1); t += step) {
        // Compute the k1 values needed by RG4
        real interim[COMPARTMENTS], k1[COMPARTMENTS], k2[COMPARTMENTS],
            k3[COMPARTMENTS], k4[COMPARTMENTS];
        ode(numVars, params, currVals, k1);
        // Generate interim values from k1 and use that to compute k2
        RK_setInterim(numVars, currVals, k1, step / 2, interim);
        ode(numVars, params, interim, k2);
        // Generate interim values from k2 and use that to compute k3
        RK_setInterim(numVars, currVals, k2, step / 2, interim);        
        ode(numVars, params, interim, k3);
        // Generate interim values from k3 and use that to compute k4
        RK_setInterim(numVars, currVals, k3, step, interim);
        ode(numVars, params, interim, k4);
        // Compute new values from intermediate values
        const real scale = step / 6.0;
        for(int i = 0; (i < numVars); i++) {
            currVals[i] += (k1[i] + (2 * k2[i]) + (2 * k3[i]) + k4[i]) * scale;
        }
    }
    // Finally, copy the results from many steps into the results array
    RK_copyArray(numVars, currVals, results);
}

void SSA_gillespie(const real step, const int MaxEqns,
                   real *rates,     constant real *Changes,
                   const real params[],
                   const int MaxComps, real* comps,
                   struct MTrand_Info* rndGen) {
    UNUSED_PARAM(step);
    // Run Gillespie's algorithm
    real time = 0;
    while (time < 1) {
        // Get the update rates based on the current compartment values
        getSSARates(MaxEqns, params, MaxComps, comps, rates);
        // Generate random probabilities and track the equation that
        // will be run in this iteration.
        int eqnToRun = -1;
        real runProb  = 1;
        // Now find the most probable equation to run at this time.
        for (int eqn = 0; (eqn < MaxEqns); eqn++) {
            if (rates[eqn] > 0) {
                const real eqnProb = -log(MTrand_get(rndGen)) / rates[eqn];
                // Track the lowest, i.e., most proabable equation to run.
                if (eqnProb < runProb) {
                    eqnToRun = eqn;
                    runProb  = eqnProb;
                }
            }
        }
        if (eqnToRun == -1) {
            continue;  // No equation found. Generate a warning?
        }
        // Process the udpates for the most probable equation by
        // applying changes to each compartment associated with this
        // set of transitions.  Note that each equation has changes to
        // all the compartments and we need to skip over earlier ones.
        // The eqnCmpIdx variable tracs the correct index in the
        // Changes 1-D array.
        for (int i = 0, eqnCmpIdx = eqnToRun * MaxComps; (i < MaxComps);
             i++, eqnCmpIdx++) {
            const real nextVal =  comps[i] + Changes[eqnCmpIdx];
            comps[i] = (nextVal > 0 ? nextVal : 0);
        }
        // Update time based on timestep
        time += runProb;
    }
}

void SSA_gillespieTauLeap(const real step, const int MaxEqns,
                          real *rates,     constant real *Changes,
                          const real params[],
                          const int MaxComps, real* comps,
                          struct MTrand_Info* rndGen) {
    // Run Gillespie's algorithm with Tau-Leaping
    // optimization. Tau-leaping optimization uses the Poisson
    // distribution support provided by MUSE.
    for (real t = 0; (t < 1); t += step) {
        // Get the update rates based on the current compartment values
        getSSARates(MaxEqns, params, MaxComps, comps, rates);
        for (int eqn = 0; (eqn < MaxEqns); eqn++) {
            if (rates[eqn] > 0.0) {
                // Get a Poisson distributed random number based on
                // the rate and step value.
                const real dailyRate = MTrand_poisson(rndGen,
                                                      rates[eqn] * step);
                // Now apply that change to each compartment
                // associated with this set of transitions.  Note that
                // each equation has changes to all the compartments
                // and we need to skip over earlier ones.  The
                // eqnCmpIdx variable tracs the correct index in the
                // Changes 1-D array.
                for (int i = 0, eqnCmpIdx = eqn * MaxComps; (i < MaxComps);
                     i++, eqnCmpIdx++) {
                    const real nextVal =  comps[i] + Changes[eqnCmpIdx] *
                        dailyRate;
                    comps[i] = (nextVal > 0 ? nextVal : 0);
                }
            }
        }
    }
}

#endif
