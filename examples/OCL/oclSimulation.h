#ifndef MUSE_OCLSIMULATION_H
#define MUSE_OCLSIMULATION_H
#include "../../include/Simulation.h"
#include "oclGVTManager.h"
#include "OCLAgent.h"
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
#include "../CL/cl.hpp"
#endif
#include <iostream>
BEGIN_NAMESPACE(muse);

using namespace cl;
using namespace std;

//forward declarations
class OCLAgent;
class oclScheduler;
class oclGVTManager;
class Communicator;
class SimulationListener;
class oclState;

class oclSimulation : public muse::Simulation{
    friend class OCLAgent;
    friend class oclState;
    friend class Simulation;
    public:

        static oclSimulation* kernel;
        
        oclSimulation(const bool usingSharedEvents = false);    

        /*
         Create a set of agents and register them with the simulation 
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
         Initializes the OpenCL kernel
         so that the kernel code can be run when needed
         without additional work
         */
        void initOCL();

        /*
         Helper function for initOCL,
         * gets the available platforms to run OpenCL kernel
         */
        Platform getPlatform();

        /*
         Helper function for initOCL,
         * gets the available devices to run OpenCL kernel
         */
        Device getDevice(Platform platform, int i, bool display);

        /*
         Helper function for initOCL,
         returns the kernel code based on whether running ssa or ode version
         of kernel code
         */
        std::string getKernel(bool ssa);

        /*
         Set up start and stop time for simulation, finish setup before
         * starting simulation including calling createAgents
         * Then calls start
         * and finalizes simulation
         */
        void simulate();

        /*
        Finalize the Simulation

        This method will cause every Agent in the system to finalize,
        and will then delete the GVTManager and finalize the
        communicator.

        \param[in] stopMPI If this flag is true, then MPI is
        finalized.  Otherwise MPI is not finalized permitting another
        simulation run using current setup.

        \param[in] delSim If this flag is true, then the derived
        simulator singleton used by this class is deleted, requiring
        full reinitialization (via call to initializeSimulation()).
         */
        void finalizeSimulation(bool stopMPI = true, bool delSim = true);

        /*
         Helper function for finalizeSimulation
         Finalizes agents and removes them from scheduler
         Deletes GVT manager, commManager, and scheduler
         */
        void finalize(bool stopMPI = true, bool delCommMgr = true);

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
        oclSimulation* initializeSimulation(int& argc, char* argv[], bool initMPI)throw (std::exception);

        /*
         Registers the given agent with the simulation
         */
        bool registerAgent(muse::OCLAgent* agent, const int threadRank);
        
        /*
        Schedule the specified event
        
	Agents use this method to schedule events that are
	not local.
     
	\param e The event you wish to schedule.  This parameter
	cannot be NULL.  The event must have a valid receive time and
	receiver agentID filled-in.
        
	\return True if scheduling is successful.
         */
        bool scheduleEvent(Event* e);
        
        /*
        Determine if the givent Agent is local 

	This method will check the registration tables to determine if
	the Agent that has the given ID is local or not.

	\param[in] id ID of the agent to check
        
	\return True if the AgentID is registered to this kernel. 
         */
        bool isAgentLocal(AgentID id) const;
        
        /*
         Convenience method to perform initialization/setup just
        before agents are initialized.
         * 
         Initializes the GVTManager and sets the gvt manger with the communicator
         */
        void preStartInit();
        
        /*Clean up old States and Events

        When this method is called, garbage collections for all the agents
        that are registered to this kernel take place.
        */
        void garbageCollect();
    
    protected:
    
        oclScheduler* scheduler;

};
END_NAMESPACE(muse);

#endif