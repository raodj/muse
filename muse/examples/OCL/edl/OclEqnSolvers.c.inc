R"(
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
    Runge-Kutta 4th order method that is used for solving a system of
    ODE equations.

    \note The equations to be solved are specified separately by
    implementing the ode(int, const double[], doube[])
    method.
*/

#ifndef real
typedef double real;
#endif

#ifndef UNUSED_PARAM
#define UNUSED_PARAM(x) (void) (x)
#endif

#ifndef COMPARTMENTS
#error For compatibility with OpenCL this solver requires COMPARTMENTS to be defined at compile time. For example make COMPARTMENTS='4' compiler flag
#endif

#ifndef __OPENCL_VERSION__
// Running on the host. We need definition for struct MTrand_Info
#include "MuseOclLibrary.h"
double log(double);
#endif

/** Interface method to be implemented by application.

    This is a forward declaration for the method that should be implemented to
    use the ODE solver.

    \param[in] numVars The number of variables/size of the
    currVals/deltas array.

    \param[in] params The parameters passed to RK_solve method.

    \param[in] currVals The current (intermediate) values.

    \param[out] deltas The resulting deltas in each variable to be
    computed by this method.
*/
void ode(const int numVars, const real params[],
         const real currVals[], real deltas[]);

/** Interface method to be implemented by application.

    This is a forward declaration for the method that should be implemented to
    use the ODE solver.

    \param[in] numVars The number of variables/size of the
    currVals/deltas array.

    \param[in] params The parameters passed to RK_solve method.

    \param[in] currVals The current (intermediate) values.

    \param[out] deltas The resulting deltas in each variable to be
    computed by this method.
*/
void getSSARates(const int numRates, const real params[],
                 const real currVals[], real rates[]);

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
        getSSARates(MaxEqns, params, comps, rates);
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
        getSSARates(MaxEqns, params, comps, rates);
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
)"
