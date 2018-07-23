#ifndef MUSE_OCL_LIBRARY_H
#define MUSE_OCL_LIBRARY_H

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

/** \file MuseOclLibrary.h

    \brief Compatibility definitions for using OpenCL random number
    generation library on the CPU for consistent operation.
*/

/** Some of the predefined compile time constants used for random
    number generation.  Since this is used to create an array further
    below, it cannot be a 'constant int' but has to be #define.
*/
#define MaxStates 624

/* An approximate middle-ish point used for random number
   generation
*/
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

// Prototype declarations for methods provided by this header
void MTrand_init(struct MTrand_Info* rndGen, unsigned long seed);
unsigned long MTrand_int32(struct MTrand_Info* rndGen);
double MTrand_get(struct MTrand_Info* rndGen);
int MTrand_poisson(struct MTrand_Info* rndGen, const double lambda);


#ifndef __OPENCL_VERSION__
// Compiling on the host
#define constant const
#endif

#endif
