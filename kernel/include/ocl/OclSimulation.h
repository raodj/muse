#ifndef MUSE_OCL_SIMULATION_H
#define MUSE_OCL_SIMULATION_H

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

#include "Simulation.h"

BEGIN_NAMESPACE(muse);

/** The top-level Heterogeneous-compute capable Simulation based on
    OpenCL (OCL).

    This class further customizes the core MUSE simulation kernel to
    enable heterogeneous-compute capable simulations.  In this
    context, "heterogeneous-compute" refers to the use of CPU and GPU
    for computation.  The GPU features of the simulation have been
    enabled using OpenCL.  Here we have chosen to use OpenCL over
    CUDA, because CUDA is limited to running only on nVidia platforms.
    On the other hand, OpenCL (limited to Version 1.1) is supported on
    all platforms.  We have consciously avoided using features from
    OpenCL 2.0 or newer releases to ensure broad portability to nVidia
    platforms.

    \note This class only overrides a few methods that need to be
    customized for OCL operations.  Rest of the operations are
    automatically performed by the base class.
*/
class OclSimulation : muse::Simulation {
    // The muse::Simulation::initializeSimulation method needs to
    // instantiate this class.
    friend class muse::Simulation;
public:
    
    /*
     * Checks whether or not to use OpenCL
     * if using OpenCL, it calls the OpenCL simulation loop
     * which will initialize OpenCL and use it for agent processing
     * if not, it calls the parent simulation start function
     */
    void start();
    
    /*
    * Helper function for initializing OpenCL,
    * returns the kernel code to process agents
    */
    std::string getKernel() = 0;
    
    /*
    * Helper function for initializing OpenCL,
    * gets the available platforms to run OpenCL kernel
    */
    Platform getPlatform();

    /*
    * Helper function for initializing OpenCL,
    * gets the available devices to run OpenCL kernel
    */
    Device getDevice(Platform platform, int i);
    
    /*
    * Run the simulation loop using OpenCL for agent processing
    */
    void oclRun();

};

END_NAMESPACE(muse);

#endif
