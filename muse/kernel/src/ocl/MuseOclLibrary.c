//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OH.
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

#ifdef __OPENCL_VERSION__

// Enable double data type
// #pragma OPENCL EXTENSION cl_khr_fp64 : enable

// The type alias used in MUSE for agentID.
typedef int AgentID;

// The type alias used in MUSE for simulation time.
typedef double Time;

//---------------------[ Random number generation ]---------------------

/**
 *
 * A Mersenne Twister random number generator compatible with MT19937
 * (algorithmically similar to the one used in C++ standard), with
 * initialization improved 2002/1/26.  Originally proposed by Takuji
 * Nishimura and Makoto Matsumoto.
 *
 * Adapted to run with OpenCL from Jasper Bedaux's 2003/1/1 (see
 * http://www.bedaux.net/mtrand/).  The generators returning floating
 * point numbers are based on a version by Isaku Wada, 2002/01/09
 *
 */

/** Some of the predefined compile time constants used for random
    number generation.  Since this is used to create an array further
    below, it cannot be a 'constant int' but has to be #define.
*/
#define MaxStates 624

/* An approximate middle-ish point used for random number
   generation */
#define m 397

/**
 * Simplified C struct to hold the key information used for generating
 * Mersenne Twister (MT) random numbers.  This structure is meant to
 * encapsulate the necessary information so that multiple/independent
 * versions of the random number generator can be used a thread-safe
 * manner.
 */
struct MTrand_Info {
    // The states used for random number generation with warp-around
    // after MaxState entries are used.
    unsigned long state[MaxStates];
    // The position in the state array
    int p;
    // Flag to indicate if values have been initialized. If this value
    // is zero, then this random number generator needs to be
    // initialized.
    int initFlag;
    // The initial random seed to be used for this random number
    // generator.
    int seed;    
};

/** The scope to be used for manipulating MTrand_Info objects.  The
    preferred scope is __private as it is the fastest memory.  When
    random numbers are used, it is best to copy the random objects
    from global to private space, use it, and copy changed states back
    to global memory space.
*/
#define RND_OCL_SCOPE __private

/**\def UNUSED_PARAM(x)

   \brief A convenient macro for specifying that a parameter to a
   method is not used.

   This macro provides a convenient approach for tagging unused
   parameters to avoid compiler warnings.  These are only meant to be
   used for parameters that are really not used.  Here is an example
   of how to use this macro:

   \code

    void oclHelper(const Time gvt) {
	UNUSED_PARAM(gvt);
	// Possibly more code goes here.
    }

   \endcode
*/
#define UNUSED_PARAM(x) (void) x


#else

// In this scenario this file is being compiled on the host.
#include "math.h"
#include "MuseOclLibrary.h"
#define RND_OCL_SCOPE

#endif

/**
   The top-level random number generation initialization function.

   \param[out] rndGen The members of this structure are initialized to
   default initial values based on the supplied seed.

   \param[in] seed The seed to be used to initialize the random number
   generator.  The default value is 5489UL.
*/
void MTrand_init(RND_OCL_SCOPE struct MTrand_Info* rndGen,
                 unsigned long seed) {
    // First, initialized the structure members to standard defaults.
    RND_OCL_SCOPE unsigned long* const state = rndGen->state;
    // Initialize the states
    state[0] = seed & 0xFFFFFFFFUL; // for > 32 bit machines
    for (int i = 1; i < MaxStates; ++i) {
        state[i] = 1812433253UL * (state[i - 1] ^ (state[i - 1] >> 30)) + i;
        // see Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier in the
        // previous versions, MSBs of the seed affect only MSBs of the
        // array state 2002/01/09 modified by Makoto Matsumoto
        state[i] &= 0xFFFFFFFFUL;  // for > 32 bit machines
    }
    // force gen_state() to be called for next random number
    rndGen->p = MaxStates;
    // Set flag to inidicate that this rnd gen has been initialized
    rndGen->initFlag = 1;
}

unsigned long MTrand_twiddle(const unsigned long u, const unsigned long v) {
    return (((u & 0x80000000UL) | (v & 0x7FFFFFFFUL)) >> 1)
        ^ ((v & 1UL) ? 0x9908B0DFUL : 0x0UL);
}

// generate new state vector
void MTrand_gen_state(RND_OCL_SCOPE struct MTrand_Info* rndGen) {
    // A shortcut reference to states for this random number generator.
    RND_OCL_SCOPE unsigned long* const state = rndGen->state;
    const int range = MaxStates - m;
    for (int i = 0; (i < range); ++i) {
        state[i] = state[i + m] ^ MTrand_twiddle(state[i], state[i + 1]);
    }
    for (int i = range; i < (MaxStates - 1); ++i) {
        state[i] = state[i + m - MaxStates] ^
            MTrand_twiddle(state[i], state[i + 1]);
    }
    state[MaxStates - 1] = state[m - 1] ^
        MTrand_twiddle(state[MaxStates - 1], state[0]);
    rndGen->p = 0; // reset position
}

// generate 32 bit random integer
unsigned long MTrand_int32(RND_OCL_SCOPE struct MTrand_Info* rndGen) {
    if (rndGen->initFlag == 0) {
        MTrand_init(rndGen, rndGen->seed);
    }
    if (rndGen->p == MaxStates) {
        MTrand_gen_state(rndGen); // new state vector needed
    }
    // gen_state() is split off to be non-inline, because it is only
    // called once in every 624 calls and otherwise irand() would
    // become too big to get inlined
    unsigned long x = rndGen->state[rndGen->p++];
    x ^= (x >> 11);
    x ^= (x <<  7) & 0x9D2C5680UL;
    x ^= (x << 15) & 0xEFC60000UL;
    return x ^ (x >> 18);
}

/** Convenience method to map a 32-bit Mersenne Twister number to the
    range 0.0---1.0 (inclusive).

    \return A uniformly distributed random number in the range
    0.0--1.0 (inclusive).
*/
double MTrand_get(RND_OCL_SCOPE struct MTrand_Info* rndGen) {
    return MTrand_int32(rndGen) * (1.0 / 4294967295.0);  // divided by 2^32
}

/** Convenience method to generate a random number based on Poisson
    distribution.  This method uses the MTrand_get to generate a
    uniform random number in the range [0, 1] to compute Poisson.

    \param[in,out] rndGen The random number generation object to be
    used.

    \param[in] lambda The mean of the Poisson distribution.

    \return A Poisson distributed (with mean lambda) random value.
*/
int MTrand_poisson(RND_OCL_SCOPE struct MTrand_Info* rndGen,
                   const double lambda) {
    const double lm_thr = exp((double) -lambda);
    int x = 0;
    double prod = 1.0;
    // Count how many times we had to generate a uniform random number
    // (that is < 1.0) number to exceed the sum.
    do {
        prod *= MTrand_get(rndGen);
        x += 1;
    } while (prod > lm_thr);
    // The number of tries becomes the Poisson random number
    return x - 1;
}

