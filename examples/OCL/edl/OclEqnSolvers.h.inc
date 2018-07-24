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

/** \file OclODE.h

    \brief Header file used only on the host (not on OpenCL) to
    include support code from OpenCL into the application to keep
    builds consistent.
*/

#include "cmath"

/** \typedef double real

    \brief A convenience typedef for a real datatype.
*/
#ifndef real
typedef double real;
#endif

#ifndef COMPARTMENTS
#error For compatibility with OpenCL this solver requires COMPARTMENTS to be defined at compile time. For example -DCOMPARTMENTS=4 compiler flag
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
extern "C"
void ode(const int numVars, const real params[],
         const real currVals[], real deltas[]);

/** The primary method for solving ODEs using Runge-Kutta 4th order
    method

    \param[in] numVars The size of the initValues and results array.

    \param[in] params This value is just passed through to ode()
    method.
    
    \param[in] initValues The initial set of values for various
    parameters in the DOE.

    \param[in] result The resulting values.

    \parma[in] step The step size to be used.
*/
extern "C"
void RK_solver(real step, const int numVars, const real params[],
               const real initValues[], real result[]);

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
extern "C"
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
extern "C"
void getSSARates(const int numRates, const real params[],
                 const real currVals[], real rates[]);

extern "C"
void SSA_gillespie(const real step, const int MaxEqns,
                   real *rates,     const real *Changes,
                   const real params[],
                   const int MaxComps, real* comps,
                   struct MTrand_Info* rndGen);

extern "C"
void SSA_gillespieTauLeap(const real step, const int MaxEqns,
                          real *rates,     const real *Changes,
                          const real params[],
                          const int MaxComps, real* comps,
                          struct MTrand_Info* rndGen);
#endif
)"
