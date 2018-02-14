#ifndef MUSE_MULTI_THREADED_SHM_SIMUALTION_MANAGER_H
#define MUSE_MULTI_THREADED_SHM_SIMUALTION_MANAGER_H

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include <mutex>
#include "mpi-mt-shm/MultiThreadedShmSimulation.h"
#include "mpi-mt-shm/MultiThreadedScheduler.h"
#include "EventRecycler.h"
#include "Agent.h"

BEGIN_NAMESPACE(muse);

/** Class to manage all the threads in the simulations.

    This class is instantiated from the main thread by
    muse::Simulation::initializeSimulation method.  Running on the
    main thread, it serves the following 2 purposes:

    <ol>
    
    <li>First it serves to create and manage other threads based on
    the number of threads specified as the command-line argument.</li>

    <li>Second it also runs as regular thread to perform various
    life-cycle actitives similar to other threads.</li>
    
    </ol>

    In order to streamline the aforementioned operations, the
    implementation has been subdivided into 2 classes.  The base class
    muse::MultiThreadedSimulation provides all the functionality
    required to perform life-cycle activities.  This class focuses on
    thread creation, agent registration, and thread management.
*/
class MultiThreadedShmSimulationManager : public MultiThreadedShmSimulation {
    // The muse::Simulation::initializeSimulation method needs to instantiate
    friend class Simulation;
public:
    /** \brief Interface method to enable derived classes to complete
        initialization of the simulator/kernel.

        <p>This method is invoked from the base class's
        initializeSimulation static method to parse the command-line
        arguments and complete initialization by instantiating
        scheduler, GVT manager, communicator, etc.  A key step in this
        method is to create multiple instances of this class that are
        eventually run as separate threads when the start method in
        this class is invoked.</p>
        
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
    virtual void initialize(int& argc, char* argv[], bool initMPI = true)
        throw (std::exception) override;

    /** \brief Finalize the Simulation

        This method will cause every Agent in the system to finalize,
        and will then delete the GVTManager and finalize the
        communicator.

        This method will also print statistics about the simulation.

        \note In this simulation class (may not apply to others), the
        finalization occurs from just 1 thread.
        
        \param[in] stopMPI If this flag is true, then MPI is
        finalized.  Otherwise MPI is not finalized permitting another
        simulation run using current setup.

        \param[in] delCommMgr If this flag is true, then the
        communication manager is deleted.  Deletion of communication
        manager may be disabled by derived classes (if it is shared or
        needs to be used for other tasks).        
    */
    virtual void finalize(bool stopMPI = true, bool delCommMgr = true) override;
    
    /** \brief Register the given Agent to the Simulation

        This method is invoked from the model code to register agents
	associated with the simulation. This method adds the agent to the
	list of shared agents between the possibly many threads that are 
	owned by this manager.

	\note Once regiestered agents will be handled by
	MUSE. Deleting of agent will be handled by MUSE. Please only
	allocate agent on the heap.
     
	\param[in] agent pointer, this is of type Agent.  This pointer
	cannot be NULL.  The agent must have a valid agent ID setup.
	The agent ID must be unique across the entire parallel
	simulation.  Once the agent is registered, then the simulation
	takes ownership of the agent object.  The agent object is
	deleted when the finalizeSimulation method is invoked.

	\param[in] threadRank, this parameter is not used for shared
	memory multi threaded simulation as agents are shared amoung
	all threads.
        
	\return True if the agent was registerd correctly.
        
	\see Agent()
	\see AgentID 
    */
    bool registerAgent(Agent* agent, const int threadRank = -1) override;

    /** \brief Start the Simulation

        This method should be called after all of the appropriate
        initialization has been completed.  Upon doing so, the
        Simulation will start.  This method is always invoked on the
        main thread (thread #0) and performs the following tasks:

        <ol>

        <li>Call preStartInit() to finish setting up the necessary
        data structures for the different threads.</li>

        <li>Starts all the threads resulting in invocation of their
        simulate() method</li>

        <li>Calls its own simulate() method in the base class to
        perform tasks for thread #0.</li>

        <li>Once the GVT has swept past the end simulation time, it
        waits for all the threads to join.</li>

        </ol>
    */
    virtual void start() override;

protected:
    /** \brief Convenience method to perform initialization/setup just
        before agents are initialized.

        This method overrides the default logic in the base classes to
        call the corresponding method on all the threads so that they
        can perform the necessary setup -- that is, instantiate the
        GVTManager for this thread.  Next, this method calls
        MultiThreadedCommunicator::registerAgents() method finish
        registering agents with all MPI processes.
    */
    virtual void preStartInit() override;

    /** Refactored utility method to create threads.

        This method is called only once from the initialize() method
        in this class.  This method creates the threads in this class,
        while setting up the necessary information to pin threads to
        specific CPUs/cores.

        \param[in,out] mtc The multi-threaded communicator shared by
        the different threads in this process.
        
        \param[in,out] mts The multi-threaded scheduler shared by
        the different threads in this process.

        \param[in] cmdArgs The command-line arguments to be passed to
        the various threads as part of initialization.
    */
    void createThreads(const int threads, MultiThreadedShmCommunicator* mtc,
                       MultiThreadedScheduler* mts,
                       std::vector<char*> cmdArgs);
    
    /** Helper method to determine the list of available CPUs to which
        threads can be pinned.

        \note The value returned by this method is meaningful only
        before the threads have been spun-up.
        
        This method enumerates the CPUs to which threads can be
        pinned.  This method internally calls pthread_getaffinity_np
        to determine the default affinity setup.

        \return A vector with list of candidate CPUs.
    */
    std::vector<int> getAvailableCPUs() const;
    
private:
    /** The list of sub-kernels that are run as independent threads.
        The entries in this list are created in the initialize()
        method.  The actual threads are created in the initAgents()
        method.
    */
    std::vector<MultiThreadedShmSimulation*> threads;

    /** Mutex to ensure only one thread at a time performs reading
        events from MPI.

        This mutex is used in the processMpiMsgs method to ensure that
        only one thread at a time reads and enqueues incoming events
        from MPI.
    */
    std::mutex mpiMutex;

    /** A temporary vector to hold incoming MPI events.

        This container is repeatedly reused in processMpiMsgs to
        obtain events from MPI via call to MultiThreadedCommunicator's
        receiveManyEvents method.  This container is reused so that we
        don't waste CPU cycles on recreating/resizing the container.
    */
    EventContainer mpiEvents;
    
    /** The only constructor for this class.

        The constructor merely initializes all the pointers and
        instance variables to their default initial value.
        
        \note The constructor is intentionally private to enfore singleton
        pattern.  Instead call, Simulation::getSimulation() method to
        obtain a valid instance of the simulation object being used to
        manage Agents (or Logical Processes).
    */
    MultiThreadedShmSimulationManager();
    
    /** The undefined copy constructor.

        This class is designed to operate as a singleton.  There
        should not be multiple instances or copies of this class.
        Consequently the copy constructor for this class is
        intentinally declared to be private and is undefined.
    */
    MultiThreadedShmSimulationManager(const Simulation &);

    /** The undefined assignment operator.

        This class is designed to operate as a singleton.  There
        should not be multiple instances or copies of this class.
        Consequently the assignment operator for this class is
        intentinally declared to be private and is undefined.
    */
    MultiThreadedShmSimulationManager& operator=(const Simulation&);

    /** The destructor.

        The destructor does not do any operations as all the
        neecessary clean-up is done in the finalize method.
     */
    virtual ~MultiThreadedShmSimulationManager();
};
    
END_NAMESPACE(muse);

#endif
