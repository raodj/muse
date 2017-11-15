#ifndef MUSE_MULTI_THREADED_SHM_SIMUALTION_H
#define MUSE_MULTI_THREADED_SHM_SIMUALTION_H

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
#include "Simulation.h"
#include "mpi-mt/MTQueue.h"

BEGIN_NAMESPACE(muse);

// Some forward declarations
class MultiThreadedShmCommunicator;
class MultiThreadedShmSimulationManager;

/** A Multi-threaded simulation class that uses shared-memory for
    intra-node communication and MPI for inter-node communication.

    This custom simulation class extends the default single-threaded
    muse::Simulation base class to enable multi-threaded simulation,
    with the following specific features:

    <ul>

    <li>This simulation mode is enabled by specifying <tt>--simulator
    mpi-mt --threads-per-node 5</tt> at the command-line of a MUSE
    compliant simulation.  The value for <tt>threads-per-node</tt>
    command-line argument typically is the same as the \c ppn value
    (as in: <tt>-lnodes=4:ppn=8</tt>) specified as part of the MPI
    executable.  In addition, the simulation must be executed using
    the <tt>--map-by</tt> MPI command-line argument as: <tt>mpixec
    --map-by ppr:1:node</tt> to assign only one process per compute
    node.</li>

    <li>Each compute-node has 1 or more threads of execution assigned
    to it.  Each thread manages the life-cycle activities of 1-or-more
    agents/LPs partitioned to it.</li>

    <li>Each thread has its own dedicated scheduler and scheduler
    queues.  The type of scheduler and the queue depends on the
    command-line parameters specified by the user.</li>
    
    <li>Exchange of events between agents occurs in one of the
    following 3 ways:
    <ol>

    <li>Exchange of events between LPs partitioned to the same thread
    is performed by direct updates to the scheduler queue, similar to
    the default, single-threaded version.</li>

    <li>Exchange of events between threads on the same compute-node is
    accomplished using shared-memory (between threads) I/O queues,
    that are treated in a similar fashion as inbound messages read
    from MPI.</li>

    <li>Exchange of events between nodes is accomplished using
    blocking MPI calls.</li>

    </ol>
    </li>

    <li>A custom communicator is used to facilitate interaction
    between threads and manage reading events from MPI.  Refer to the
    documentation on the custom communicator for specifics of its
    operations.</li>

    <li>The standard Mattern GVT manager is used to circulate tokens
    around all the threads.  Sure, GVT calculation could be more
    efficient using a customized shared-memory Local-GVT calculation;
    that is future work.</li>
    
    </ol>    
    </ul>
    
    <p>This class is the heart of various simulation activities in
    MUSE. All core operation that the engine handles are done from
    this class. This class implements the Singleton pattern and should
    NOT be used as a super class to derive from. In addition, the
    client should not try and create an instance using the
    constructor. use the getSimulator() method which will return a
    pointer to a Simulation object.</p>   
    
    \note refer to each method documentation, including base-class
    documentation for more details on the features provided.
*/
class MultiThreadedShmSimulation : public muse::Simulation {
    // GVTManager needs to be able to call garbageCollect()
    friend class GVTManager;
    // The manager needs to create instances of this class
    friend class MultiThreadedShmSimulationManager;
public:        
    /** \brief Get the number of threads per processes

        This method can be used to determine the number of threads per
        MPI-process.  This value is more meangiful when multi-threaded
        simulator is used (see --threads-per-node command-line
        argument in MultiThreadedSimulation class).
        
        \return The number of threads per parallel/MPI processes.
        (this value is not the total number of threads in the
        simulation).
    */    
    virtual unsigned int getNumberOfThreads() const { return threadsPerNode; }
    
protected:
    /** The default constructor for this class.

        The constructor merely initializes all the instance variables
        to default (or invalid) initial values.  The constructor does
        not perform any further tasks.

        \param[in] simMgr The manager thread that logically owns this
        object.  This pointer is used to invoke processMpiMsgs() in
        the manager object.
        
        \param[in] thrID The logical, local zero-based index
        associated with this class.  This value is in the range 0 <=
        thrID < threadsPerNode.

        \param[in] globalThrID The global thread ID associated with
        this thread.  This value is unique across the process and is
        computed as: (MPI_rank * threadsPerNode + thrID)

        \param[in] threadsPerNode The total number of threads running
        on this node (with thread IDs: 0, 1, ..., threadsPerNode-1).
    */
    MultiThreadedShmSimulation(MultiThreadedShmSimulationManager* simMgr,
                            int thrID = 0, int globalThrID = 0,
                            int threadsPerNode = 1);

    /** The destructor.

        The destructor does not perform any tasks and is just a place
        holder.  Necessary clean-up is done in the finalize() method
        to enable running multiple simulations (by calling
        muse::Simulation::finalizeSimulation(false, false))
    */
    virtual ~MultiThreadedShmSimulation();
    
    /** \brief Interface method to enable derived classes to complete
        initialization of the simulator/kernel.

        <p>This method is invoked from the base class's
        initializeSimulation static method.  A key step in this method
        is to create multiple instances of this class that are
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
        throw (std::exception);

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
        
    /** Refactored utility method to parse command-line arguments
        pertinent to the kernel.

        This method is called just once from the initialize method to
        parse out the command-line arguments associated with the
        kernel.  This method consumes the following command-line
        arguments and passes the rest of them to the base cass for
        further parsing:

        <ul>

        <li>The type of MT-queue to be used (\c --mt-queue).</li>
        
        </ul>

	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.        
    */
    virtual void parseCommandLineArgs(int &argc, char* argv[]) override;

    /** \brief Convenience method to perform initialization/setup just
        before agents are initialized.

        This method is invoked from the main thread from
        MultiThreadedSimulationManager to initialize the threads prior
        to simulation.  This method first let's the base class perform
        necesssary initialization.  It then overrides the GVT
        manager's rank to a thread-based rank.
    */
    virtual void preStartInit() override;

    /** Set pointer to common/shared multi-threaded communicator.

        This method is typically invoked from
        MultiThreadedSimulationManager::initialize method to set the
        pointer to the communicator to be used.  The communicator is
        shared between multiple threads.  Consequently, it is aware of
        multiple threads and routes events accordingly.  This method
        sets both MultiThreadedSimulation::mtCommMgr and
        Simulation::commMgr.
    */
    void setCommManager(MultiThreadedShmCommunicator* mtc);

    /** Performs the core simulation operation for each thread.

        This method is invoked on each thread associated with the
        simulation.  This method performs the following tasks:

        <ol>

        <li>It initializes all the agents partitioned to this
        thread.</li>

        <li>Repeatedly performs the following tasks until GVT sweeps
        past the end simulation time:
        
        <ul>

        <li>Check and processes any incoming events/messages from
        other threads or delivered via MPI.</li>

        <li>Processes pending events in the event queue.</li>
        
        </ul>
        
        </li>
        
        </ol>
    */
    virtual void simulate();

    /** \brief Schedule the specified event.
        
	Agents actually use this method to schedule all events.
	Events for agents partitioned to this thread are inserted
	directly into the scheduler.  Events for agents on other
	threads or remote processes thread are passed via the GVT
	manager to the communicator for dispatch.

	\note Users should not be using this method. If used, will
        cause undefined behavior.  Use Agent::scheduleEvent method to
        avoid potential problems.
     
	\param e The event you wish to schedule.  This parameter
	cannot be NULL.  The event must have a valid receive time and
	receiver agentID filled-in.
        
	\return True if scheduling is successful.
        
	\see Event()
    */
    virtual bool scheduleEvent(Event* e) override;

    /** Method to process incoming messages from other threads.

        This method is periodically invoked from the core simulation
        loop to process all events currently present in incomingEvents
        queue.  The events are from other threads or remote processes.
    */
    virtual void processIncomingEvents();

    /** Read messages (if any) from MPI and add them to incomingEvent
        queues of various threads.

        This method is repeatedly invoked from the core simulation
        loop to read messages via MPI.  This is a polymorphic call
        that occurs only on sub-threads.  This method dispatches the
        call to MultiThreadedSimulationManager::processMpiMsgs().  

        \note Events read by this method could be destined to any of
        the threads on this nodes.  This method appropriately
        dispatches them to the incoming event queues of various
        threads for final processing.
    */
    virtual void processMpiMsgs() override;
    
protected:
    /** The number of threads to be spun-up for each MPI process.

        This value is initialized to 1 by default.  The actual value
        to use is in the initialize() method when the command-line
        arguments are parsed.
    */
    int threadsPerNode;

    /** The local zero-based logical index value associated with this
        thread.

        This number is a zero-based index value associated with this
        thread.  This value is in the range 0 <= threadID <
        threadsPerNode.
    */
    const int threadID;

    /** The zero-based logical index value associated with this
        thread.

        This number is similar to an MPI rank.  It is computed as:
        MPI_rank * threadsPerNode + threadID.  Consequently, each
        thread gets a unique value across the processes.  This is the
        value that is used by communicator to appropriately route
        events.
    */
    int globalThreadID;
    
    /** A list of incoming events from other threads and processes.

        The list of incoming events that will be processed by this
        thread.  This list is multi-threading (MT) safe -- that is,
        multiple threads can simultaneously operate on this list
        without experiencing race conditions.  The list is initialized
        in the parseCommandLineArgs method.
    */
    MTQueue* incomingEvents;
    
private:
    /** The undefined copy constructor.

        This class is not copyable -- that is duplicate copies should
        not be made.  Consequently the copy constructor for this class
        is intentinally declared to be private and is undefined.
    */
    MultiThreadedShmSimulation(const Simulation &);

    /** The undefined assignment operator.

        This class is not assignable -- that is data cannot be
        directly updated from another copy. Consequently the
        assignment operator for this class is intentinally declared to
        be private and is undefined.
    */
    MultiThreadedShmSimulation& operator=(const Simulation&);

    /** The multi-threading aware communicator.

        This communicator is shared between multiple threaded.  It is
        created in the
        muse::MultiThreadedSimulationManager::initialize method.  The
        pointer is filled-in via call to setCommManager method in this
        class.  Note that this pointer is exactly the same as pointer
        set in the base class's muse::Simulation::commManager.  We
        maintain a pointer to the derived class so that we don't have
        to do any typecasting etc.
    */
    MultiThreadedShmCommunicator* mtCommMgr;

    /** The simulation manager that owns this simulation.

        This pointer is internally saved and used to dispatch calls to
        processMpiMsgs() method.  The value is set in the constructor
        and is never changed during the life time of this object.
    */
    MultiThreadedShmSimulation* const simMgr;
};

END_NAMESPACE(muse);

#endif
