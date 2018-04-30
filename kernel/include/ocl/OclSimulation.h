#ifndef MUSE_OCLSIMULATION_H
#define MUSE_OCLSIMULATION_H

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
#include "GVTManager.h"
#include "oclAgent.h"
#include "Event.h"
#include "State.h"
#include "DataTypes.h"
#include "Avg.h"
#include "ArgParser.h"
#include "oclScheduler.h"
#include "oclState.h"
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#include <random>
#include <typeinfo>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include "CL/cl.hpp"
#endif
#include <iostream>
BEGIN_NAMESPACE(muse);

// Use real for all state data so users can redefine real as anything needed
typedef float real;

// forward declarations
class OclAgent;
class OclScheduler;
class Communicator;
class SimulationListener;
class OclState;

class OclSimulation : public Simulation{
    friend class OclAgent;
    friend class OclState;
    friend class Simulation;
    
    public:
        // class variables
        std::vector<OclAgent*> oclAgents;
        bool ode;
        bool oclAvailable;
        int compartments;
        float step;
        Time lastLGVT;
        int maxWorkGroups;
        cl::CommandQueue queue;
        cl::Kernel run;
        cl::Buffer buffer_B;
        cl::Buffer buffer_random;
        cl::Buffer buffer_compartments;
        
       /** The constructor for this class.
        * 
        * The constructor merely initializes all the instance variables
        * to default (or invalid) initial values.  The constructor does
        * not perform any further tasks.
        * 
        * \param[in] usingSharedEvents Flag to indicate if events are
        * directly shared between threads on a process.
        */
        explicit OclSimulation(const bool usingSharedEvents = false);    
        
        /**
         * Parses command line arguments that are needed for the simulation
         * 
         * \param argc[in,out] The number of command line arguments.
         * This is the value passed-in to main() method.  This value is
	 * modifed if command-line arguments are consumed.
         *
	 * \param argv[in,out] The actual command line arguments.  This
	 * list is modifed if command-line arguments are consumed by the
	 * kernel.
         */
        void parseClassVars(int& argc, char* argv[]) throw(std::exception);
    
        /**
         * Starts the simulation
         * 
         * Called after all of the appropriate initialization is complete
         * This function has minor changes from the base Simulation class
         * start method to allow the use of OpenCL to process agents
         * concurrently.
         */
        void start();
        
    protected:

        /** process the next set of events (if any) associated with 1 agent.
        *
        * This method is repeatedly invoked from the core simulation
        * loop (in start() method) to process the next set of events for
        * 1 agent. 
        *
        * \return If event(s) were scheduled/processed this method
        * returns true.  If no events were processed (either because of
        * time-window restrictions or there were no events) then this
        * method returns false.
        */
        bool processNextEvent();

        /*
         * Helper function for initOCL,
         * Gets the available platforms to run OpenCL kernel
         * 
         * \return If it finds a valid platform, it returns 
         * the platform that will be used by the OpenCL kernel code
         */
        cl::Platform getPlatform();

        /**
         * Helper function for initOCL,
         * Gets the available devices to run OpenCL kernel
         * 
         * \return if it finds valid devices, it returns the 
         * device to run OpenCL kernel code with
         */
        cl::Device getDevice(cl::Platform platform, int i, bool display);
        
        /**
         * Initialize the OpenCL platform, device, context, and queue
         * then sets the class variables to be used later in processAgents
         * 
         * \param agent. The agent is used to get the OpenCL kernel code
         * that will be used to concurrently process agents.
         * 
         * \param maxGlobal. The maximum number of agents to run concurrently
         * with OpenCL. This number is limited by the maximum global size
         * of the device being used.
         */
        virtual void initOCL(oclAgent* agent, int maxGlobal); 
        
        /*
         * Run OpenCL kernel code to process agents. 
         * 
         * \param maxGlobal. The maximum number of agents to run concurrently
         * with OpenCL. This number is limited by the maximum global size
         * of the device being used.
         */
        virtual void processAgents(int maxGlobal);
        
        /** Initializes simulation kernel using the base class
        
	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.

        \param initMPI[in] Flag to indicate if MPI needs to be
        reinitialized.  This flag is set to false if the simulation is
        simply being repeated and initialization of MPI is not
        necessary.
        */
        void initialize(int& argc, char* argv[], bool initMPI) 
            throw(std::exception);
        
        /** \brief Convenience method to perform initialization/setup just
        * before agents are initialized.
        * Uses the base Simulation class for most initialization functionality.
        */
        void preStartInit();

};
END_NAMESPACE(muse);

#endif