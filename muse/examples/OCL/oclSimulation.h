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
#include <iostream>
#include <random>
#include <typeinfo>
#include <functional>
#include <algorithm>
#include <chrono>
#include <iterator>
#ifdef __APPLE__
#include <OpenCL/cl.hpp>
#else
#include "CL/cl.hpp"
#endif
#include <iostream>
BEGIN_NAMESPACE(muse);

using namespace std;

// Use real for all state data so users can redefine real as anything needed
typedef float real;

//forward declarations
class oclAgent;
class oclScheduler;
class Communicator;
class SimulationListener;
class oclState;

class oclSimulation : public Simulation{
    friend class oclAgent;
    friend class oclState;
    friend class Simulation;
    public:
                
       /** The constructor for this class.

        The constructor merely initializes all the instance variables
        to default (or invalid) initial values.  The constructor does
        not perform any further tasks.

        \param[in] usingSharedEvents Flag to indicate if events are
        directly shared between threads on a process.
    */
        oclSimulation(const bool usingSharedEvents = false);    
        
                /*
         Complete initialization of the Simulation

        Once the simulation instance is created, it must be full
        initialized.  Afterwards, important instance variables such as the
        SimulatorID are set.
        
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

        \return A pointer to the newly created simulation object.
        This pointer can also be obtained via call to getSimulator()
        method in this class.
         */
        Simulation* initializeSimulation(int& argc, char* argv[], bool initMPI)throw (std::exception);
        
        /*
         Set up start and stop time for simulation, finish setup before
         * starting simulation including calling createAgents
         * Then calls start
         * and finalizes simulation
         */
        void simulate();
        
            /** Convenience method to determine is events are being shared
        between multiple threads on this process.

        This is a convenience method that can be used to determine if
        events are directly shared between two threads on this
        process.  In such scenarios, two different reference counters
        (one for send thread and another for receive thread) are
        internally used to manage and recycle events.

        \return This method returns true if events are shared between
        threads.  Otherwise this method returns false.
        */
        inline bool usingSharedEvents() const { return doShareEvents; }
    
    protected:
        // class variables
        std::vector<oclAgent*> oclAgents;
        bool ode = true;
        bool oclAvailable = false;
        int compartments = 4;
        real* values;
        Time lastLGVT;
        float stopTime = 25;
        int rows = 300;
        int cols = 300;
        int popSize;
        int expSize;
        float step;
        int maxWorkGroupSize;
        int maxWorkGroups;
//        static Simulation* kernel;
        oclScheduler* scheduler;
        cl::CommandQueue queue;
        cl::Kernel run;
        cl::Buffer buffer_B;
        cl::Buffer buffer_random;
        cl::Buffer buffer_compartments;

        /*
         * Create a set of agents and register them with the simulation 
         * This function uses simulation class variables to determine
         * specifics, like total number of agents.
         */
        void createAgents(); 
        
    /** process the next set of events (if any) associated with 1 agent.

        This method is repeatedly invoked from the core simulation
        loop (in start() method) to process the next set of events for
        1 agent. 

        \return If event(s) were scheduled/processed this method
        returns true.  If no events were processed (either because of
        time-window restrictions or there were no events) then this
        method returns false.
    */
        bool processNextEvent();

        /*
         * Starts the simulation
         * 
         * Called after all of the appropriate initialization is complete
         */
        void start();

        /*
         Helper function for initOCL,
         * gets the available platforms to run OpenCL kernel
         */
        cl::Platform getPlatform();

        /*
         Helper function for initOCL,
         * gets the available devices to run OpenCL kernel
         */
        cl::Device getDevice(cl::Platform platform, int i, bool display);
        
        /*
         * Initialize the OpenCL platform, device, context, and queue
         */
        void initOCL(oclAgent* agent);
                
        
        /*
         Run the main simulation loop using opencl
         this avoids opencl scoping problems.
         */
        void simLoop();
        
        /*
         * Run opencl kernel code to process agents
         */
        void processWithOCL(int maxGlobal);
        
        void initialize(int& argc, char* argv[], bool initMPI) 
            throw (std::exception);
        void preStartInit();
        void finalize(bool stopMPI, bool delCommMgr);
        void garbageCollect();
};
END_NAMESPACE(muse);

#endif