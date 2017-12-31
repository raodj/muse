#ifndef MUSE_MULTI_THREADED_SIMUALTION_H
#define MUSE_MULTI_THREADED_SIMUALTION_H

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
#include "SpinLockThreadBarrier.h"
#include "mpi-mt/MTQueue.h"
#include "Avg.h"
#include "NumaMemoryManager.h"

BEGIN_NAMESPACE(muse);

// Some forward declarations
class MultiThreadedCommunicator;
class MultiThreadedSimulationManager;

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
class MultiThreadedSimulation : public muse::Simulation {
    // The manager needs to create instances of this class
    friend class MultiThreadedSimulationManager;
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

        \param[in] usingSharedEvents Flag to indicate if events are to
        be directly shared between sending and receiving threads on
        this process.

        \param[in] cpuID An optional CPU ID to which the thread is to
        be pinned.  This value is used to set the affinity for this
        thread.  This value is determined by the
        MultiThreadedSimulationManager::start() method.  The CPU ID is
        chosen such that it also honors PBS job settings as well.  If
        the cpuID is -1, then processor affinity is not setup.
    */
    MultiThreadedSimulation(MultiThreadedSimulationManager* simMgr,
                            int thrID = 0, int globalThrID = 0,
                            int threadsPerNode = 1,
                            bool usingSharedEvents = false,
                            int cpuID = -1);

    /** The destructor.

        The destructor does not perform any tasks and is just a place
        holder.  Necessary clean-up is done in the finalize() method
        to enable running multiple simulations (by calling
        muse::Simulation::finalizeSimulation(false, false))
    */
    virtual ~MultiThreadedSimulation();
    
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
    void setCommManager(MultiThreadedCommunicator* mtc);

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

    /** \brief Clean up old States and Events

        This method overrides the default implementation in the base
        class.  First it passess control to the base class to perform
        the standard garbage collection process.  Next, this method
        calls EventRecycler::processPendingDeallocs() to try to
        reclaim pending events after a garbage collection cycle has
        completed.  This is done here because EventRecycler::
        processPendingDeallocs method is a time consuming one
        (particularly when there are many events pending) and each
        iteration through it has to be successful.

        \see EventRecycler::processPendingDeallocs()
     */
    virtual void garbageCollect() override;
    
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

        \return This method returns the number of MPI messages that
        were received and processed.  If no messages were received,
        this method returns zero.
    */
    virtual int processMpiMsgs() override;

    /** Overridable method to report additional local statistics (from
        derived classes) at the end of simulation.

        This method was introduced to enable derived classes report
        additional statistics.  This method is called from the
        reportStatistics() method in this class.  The statistics from
        the derived class must be written to the supplied output
        stream.

        \param[out] os The output stream to which statistics are to be
        written. By default statistics are reported to std::cout.
    */
    virtual void reportLocalStatistics(std::ostream& os = std::cout) override;

    /** Setup CPU affinity for this thread.

        This method is called only once on each thread from the
        simulate() method.  This method uses the cpuID setup for this
        thread to setup process affinity for this thread.  In
        addition, this method also sets the NUMA node associated with
        the CPU to which the thread is setup.
    */
    void setCpuAffinity() const;


    /** Convenience method to get the NUMA node ID for a given CPU id.

        This method is a convenience method used to streamline the
        code for enabling NUMA-aware memory management.  This method
        returns the NUMA node ID for a given logical CPU id by calling
        numa_node_of_cpu().

        \return The NUMA node for the CPU.  If NUMA is not available,
        this method always returns -1.
    */
    int getNumaNodeOfCpu(const int cpu) const;
    
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

    /** Refactored utility method to make copy of an event honoring
        NUMA and other runtime flags/settings.

        This is a refactored utility method used to make copy of
        events going across thread boundaries.  This method is
        currently called only from MultiThreadedSimulation's
        scheduleEvent method.  This method is used only when
        usingSharedEvents is set to \c false.  This method make an
        identical clone of the given event.

        \param[in] src The source event to be cloned. This parameter
        cannot be NULL.

        \param[in] receiver The receiving agent ID used for cloning
        event (if NUMA is not being used or when sharing events).
        
        \param[in] destThrID The global rank of the destination thread
        to which the event is to be sent.  This value modulo
        threadsPerNode gives the local thread ID.

        \return A clone of the event with memory suitably allocated.
    */
    muse::Event* cloneEvent(const muse::Event* src,
                            const muse::AgentID receiver) const;
    
private:
    /** The undefined copy constructor.

        This class is not copyable -- that is duplicate copies should
        not be made.  Consequently the copy constructor for this class
        is intentinally declared to be private and is undefined.
    */
    MultiThreadedSimulation(const Simulation &);

    /** The undefined assignment operator.

        This class is not assignable -- that is data cannot be
        directly updated from another copy. Consequently the
        assignment operator for this class is intentinally declared to
        be private and is undefined.
    */
    MultiThreadedSimulation& operator=(const Simulation&);

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
    MultiThreadedCommunicator* mtCommMgr;

    /** The simulation manager that owns this simulation.

        This pointer is internally saved and used to dispatch calls to
        processMpiMsgs() method.  The value is set in the constructor
        and is never changed during the life time of this object.
    */
    MultiThreadedSimulation* const simMgr;

    /** A shared barrier to facilitate coordination of threads as they
        wind-up.

        This barrier is used to wait for threads to finish-up their
        main loops so that any pending events can be correctly
        recycled.
    */
    static SpinLockThreadBarrier threadBarrier;

    /** Reference to the main threads' pending deallocations.

        This list always referes to the pending deallocations list on
        the main thread.  This list is used for the following reason
        -- Pending event deallocations on various threads cannot be
        fully reclaimed until all agents are finalized (and they
        relinquish references to events in their internal queues).
        Consequently, at the end of the
        MultiThreadedSimulation::simulate() method, each thread (other
        than the main thread) adds its pending events to this list.
        This list is finally cleaned-up in the main thread.
    */
    EventContainer& mainPendingDeallocs;

    /** Track the fraction of events reclaimed from EventRecycler's
        pendingDeallocs list at end of each GC cycle.  This counter is
        updated in garbageCollect method in this class.  This value
        essentially indicates the number of times EventRecycler::
        processPendingDeallocs() method was called and the fraction of
        events that were successfully reclaimed.  If this value is low
        then we need fewer calles to the method.
    */
    Avg deallocsPerCall;
    
    /** Command-line argument to set the desired fraction of events
        cleared from pendingDeallocs list for each GC cycle.  This
        value is a fraction in the range 0.01 to 1.0.  It cannot be
        set to zero. This value is used to double the deallocRate
        until the desired deallocFrac is achieved.  If the fraction of
        deallocated events is more than 1.5 times this value then the
        deallocRate is halved (but not below 1).
    */
    double deallocThresh;

    /** The current rate at which processPendingDeallocs() should be
        called.  This value is updated after each call to
        processPendingDeallocs() method to reach the specified
        deallocFrac threshold value, the following manner:

        \code

        const double fracRecovered = EventRecycler::processPendingDallocs();
        if (fracRecovered < deallocThresh) {
            deallocRate *= 2;
        } else if (fracRecovered > (deallocThresh * 1.5)) {
            deallocRate = std::max(1, deallocRate / 2);
        }
    */
    int deallocRate;

    /** This is the counter used to determine if
        processPendingDeallocs() method should be called.  This
        counter is initialized to deallocRate.  It is decremented in
        each call to garbageCollect() method.  If this counter reaches
        zero, then processPendingDeallocs() is called and it is reset
        back to deallocRate.
    */
    int deallocTicker;

    /** Average number of events processed as a single batch from the
        incomingEvents queue.

        This stats object tracks the average number of events
        read/processed from the shared incomingEvents queue.  This
        statistic does not track the calls that returned zero
        messages.
    */
    Avg shrQevtCount;

    /** Counter to track the number of times the incomingEvents
        queue's removeAll method was called.

        This instance variabe tracks the number of times the
        incoming queue was checked by this thread.
    */
    size_t shrQcheckCount;

    /** List of incoming events read from incomingEvents queue.

        This is a temporary container that is re-used to read incoming
        events over the shared mult-threaded queue associated with
        this container.  This container is used just in the
        processIncomingEvents method.
    */
    EventContainer shrEvents;

    /** The CPU ID setup for this thread.

        The CPU ID setup for this thread to establish CPU affinity.
        This value is used when the threads actually start running.
        If this value is -1, then CPU affinity is not setup.
    */
    int cpuID;

    /* Flag to indicate if NUMA usage is completely disabled.

       If this flag is true, then NUMA operations are never used.  If
       this flag is <u>\c false and usingSharedEvents is also \c
       false</u> then NUMA is used to allocate memory for events going
       across thread boundaries.  This ensures that sending threads
       have copy on their NUMA node while receiving thread has copy in
       its NUMA node to ensure good performance.

       \see cloneEvent
    */
    bool noNuma;
    
    /** List of NUMA node IDs for each thread to be used by NUMA-aware
        memory manager.

        This is a static (i.e., shared by all threads) list that
        contains the NUMA-node ID (0, 1, 2, ...) for each thread.
        Note that many threads can have have same NUMA-node ID based
        on the number of cores on each CPU.  Entries are added to this
        vector by MultiThreadedSimulationManager::createThreads
        method.  The list is passed onto the NUMA-aware EventRecycler
        to allocate memory appropriate NUMA nodes.
    */
    static std::vector<int> numaIDofThread;

#if USE_NUMA == 1
    /** Reference to main Numa memory manager to add list of allocated
        pages to finally free at end of simulation.

        This reference always referes to the NumaMemoryManager on the
        main thread.  This object is used for the following reason --
        Event deallocations on various threads cannot be fully
        reclaimed until all agents are finalized (and they relinquish
        references to events in their internal queues).  Consequently,
        at the end of the MultiThreadedSimulation::simulate() method,
        each thread (other than the main thread) adds its NUMA blocks
        to the main numa manager. This list is finally cleaned-up in
        the main thread.
    */
    NumaMemoryManager& mainNumaManager;

    /** String to temporarily hold thread-local NUMA memory management
        statistics.

        The thread-local NUMA memory manager goes out of scope when
        the thread ends at the end of the simulate method.  However,
        statistics are reported after the thread ends.  Consequently,
        this instance variable maintains a copy of the NUMA statistics
        to be reported when the reportLocalStatistics method is
        called.
     */
    std::string numaStats;

#endif
};

END_NAMESPACE(muse);

#endif
