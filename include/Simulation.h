#ifndef MUSE_SIMUALTION_H
#define MUSE_SIMUALTION_H

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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include "Agent.h"
#include "Event.h"
#include "State.h"
#include "DataTypes.h"
#include "Avg.h"

#include <mpi.h>

BEGIN_NAMESPACE(muse);

// Forward declaration to make compiler happy and fast
class GVTManager;
class Scheduler;
class Communicator;
class SimulationListener;
class OclScheduler;

/** The Simulation Class.
 
    This is the heart of muse. All core operation that the engine
    handles are done from this class. This class implements the
    Singleton pattern and should NOT be used as a superclass to derive
    from. Also the client should not try and create an instance using
    the constructor. use the getSimulator() method which will return a
    pointer to a Simulation object. Once you have all your agents and
    their events in place you can get the simulation gwoing with the
    following three easy steps:
    
    <ol>
    
    <li> Get a simulation object via the getSimulator() method.</li>
    
    <li> Register all your agents by repeatedly invoking the
    register() method.</li>
    
    <li>Finally get the simulation stared via the start() method. To
    stop the simulation early just call the stop() method.</li>
    
    </ol>
    
    
    \note refer to each method documentation for more details on the
    features provided.
    
*/
class Simulation {
    // GVTManager needs to be able to call garbageCollect()
    friend class GVTManager;
    friend class Agent;
    friend class Scheduler;
    friend class OclSimulation;
public:
    /** \brief Complete initialization of the Simulation

        Once the simulation instance is created, it must be full
        initialized.  Notably, this includes initializing
        MPI. Afterwards, important instance variables such as the
        SimulatorID are set.

        \note This method may throw an exception if errors are
        encountered during initialization.
        
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
    static Simulation* initializeSimulation(int& argc, char* argv[],
                                            bool initMPI = true);

    /** \brief Finalize the Simulation

        This method will cause every Agent in the system to finalize,
        and will then delete the GVTManager and finalize the
        communicator.

        This method will also print statistics about the simulation.

        \param[in] stopMPI If this flag is true, then MPI is
        finalized.  Otherwise MPI is not finalized permitting another
        simulation run using current setup.

        \param[in] delSim If this flag is true, then the derived
        simulator singleton used by this class is deleted, requiring
        full reinitialization (via call to initializeSimulation()).
    */
    static void finalizeSimulation(bool stopMPI = true, bool delSim = true);
        
    /** \brief Get the Simulator ID
     
	\return The ID of the simulation kernel
        
	\see SimulatorID
    */
    virtual inline SimulatorID getSimulatorID() const { return myID; }

    /** \brief Get the total number of processes

        \return The total number of parallel/MPI processes
        collaborating in this simulation.  This value does not
        indicate the number of threads in the simulation.
    */
    virtual inline unsigned int getNumberOfProcesses() const
    { return numberOfProcesses; }

    /** \brief Get the number of threads per processes

        This method can be used to determine the number of threads per
        MPI-process.  By default this method returns 1.  This value is
        more meangiful when multi-threaded simulator is used (see
        --threads-per-node command-line argument in
        MultiThreadedSimulation class).
        
        \return The number of threads per parallel/MPI processes.
        (this value is not the total number of threads in the
        simulation).
    */    
    virtual unsigned int getNumberOfThreads() const { return 1; }
    
    /** \brief Register the given Agent to the Simulation

        Once you design and create an agent for your simulation, use
	this method to register the agent to the simulator. For
	example a client could check the SimulatorID and with a switch
	statement register agents to different simulators on different
	processes. See examples.

	\note Once regiestered agents will be handled by
	MUSE. Deleting of agent will be handled by MUSE. Please only
	allocate agent on the heap.
     
	\param[in] agent pointer, this is of type Agent.  This pointer
	cannot be NULL.  The agent must have a valid agent ID setup.
	The agent ID must be unique across the entire parallel
	simulation.  Once the agent is registered, then the simulation
	takes ownership of the agent object.  The agent object is
	deleted when the finalizeSimulation method is invoked.

        \param[in] threadRank An optional parameter to specify a
        specific thread in this process to which this agent should be
        partitioned.  The default value of -1 causes agents to be
        assigned to threads in a round-robin manner.
        
	\return True if the agent was register correctly.
        
	\see Agent()
	\see AgentID 
    */
    virtual bool registerAgent(Agent* agent, const int threadRank = -1);


    /** \brief Get all Agents registered to the simulation
        
        \return An AgentContainer with all registered agents
        \see AgentContainer
    */
    inline const AgentContainer& getRegisteredAgents() const { return allAgents; }
    
    /** \brief Get the only, singleton Simulation object.

        The simulation class implements a singleton pattern. Call this
	method to get a pointer to the simulation kernel.
	
	\return A pointer to the one-and-only Simulation
    */
    static Simulation* getSimulator() { return kernel; }
    
    /** \brief Start the Simulation

        This method should be called after all of the appropriate
        initialization has been completed.  Upon doing so, the
        Simulation will start.
    */
    virtual void start();
        
    /** \brief Sets the simulation start time.

        Keep in mind that the simulation does not have to start at
	time Zero (0) -- each simulation object could start at
	different times. Warning, if you decided to start at different
	times, it could cause rollbacks.

	\param sTime The start time
	\see Time
    */
    virtual void setStartTime(Time sTime) { startTime = sTime; }

    /** \brief Stop the simulation

        Currently unimplemented.

        This method stops the Simulation.  There should be no need to
        use this method, use setStopTime() instead.
        
	\todo Implement this. Currently does nothing.
    */
    virtual void stop();
        
    /** \brief Sets the simulation stop time. 

        Ideally this method must be invoked (on all parallel
        instances) prior to starting the simulation.  Behavior of
        invoking this method during simulation is unspecified (i.e.,
        it may or may-not work depending on your
        simulation/application).
        
	\param eTime The stop time.
        
	\see Time
    */
    virtual void setStopTime(Time eTime) { endTime = eTime;}

    /** \brief Set the GVT Delay Rate
        
	Different Simulation models perform better with different
	rates of GVTEstimation.  If you have a large number of agents
	its better to make this a smaller number between (1-50)
	otherwise it would make your simulation faster to minimize
	garbage collection.  Typically this method is invoked (on all
	parallel processes) prior to commencement of simulation.
	Behavior of invoking this method during simulation is
	unspecified (i.e., it may or may-not work depending on your
	simulation/application).

	\note The default is rate is 100
        
	\param rate The rate at which to delay GVT estimation (rate
	representing timesteps)
    */
    virtual void setGVTDelayRate(int rate) { gvtDelayRate = rate;}
    
    /** \brief Determine if the givent Agent is local 

	This method will check the registration tables to determine if
	the Agent that has the given ID is local or not.

	\param[in] id ID of the agent to check
        
	\return True if the AgentID is registered to this kernel.
    */
    inline bool isAgentLocal(AgentID id) const;

    /** \brief Check if a pair of agents are local to a given
	process/thread.
	
	\param[in] id1 The agent id to be used for checks.

        \param[in] id2 The agent id to be used for checks.

        \return True if the two agents are registered on the same
        process/thread.  Otherwise this method returns false.
        
	\see AgentID
    */
    bool isAgentLocal(const AgentID id1, const AgentID id2) const;
    
    /** \brief Get the start-time of the Simulation

        \return the start time of this simulation
	\see Time
    */
    virtual inline Time getStartTime() const { return startTime; }

    /** \brief Get the end-time of the Simulation
        
	\return the end time of this simulation
	\see Time
    */
    virtual inline Time getStopTime() const { return endTime; }

    /** \brief Set a callback class to report major simulation phases.

	This method must be used to set an listener to be used to
	report major occurrences within the simulator.  Any previous
	listener is discarded and a new listener is set. If the
	parameter is NULL, then the listeners are cleared.

	\param[in] listener The listener to be used to report major
	phases during the course of simulation.  No special checks are
	made on the pointer.

	\note It is the caller's responsibility to delete the listener
	as needed once it is removed (by calling setListener(NULL);)
    */
    virtual void setListener(SimulationListener* listener);

    /** \brief Get the Global Virtual Time
        
        \return The GVT
        \see Time
    */
    virtual Time getGVT() const;

    /** \brief Get the LGVT
        
        The LGVT is the next time that will become the LVT.

	\return The LGVT
	\see Time
     */
    virtual Time getLGVT() const;

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

    /** Utility method to determine if this simulation kernel has
        Heterogeneous Computing (HC) capabilty.

        \return This method returns true if the kernel has HC
        capabilities.  By default this method returns false.
    */
    virtual bool hasHCsupport() const { return false; }
    
protected:
    /** The default constructor for this class.

        The constructor merely initializes all the instance variables
        to default (or invalid) initial values.  The constructor does
        not perform any further tasks.

        \param[in] usingSharedEvents Flag to indicate if events are
        directly shared between threads on a process.
    */
    Simulation(const bool usingSharedEvents = false);

    /** The destructor.

        The destructor does not perform any tasks and is just a place
        holder.  Necessary clean-up is done in the finalize() method
        to enable running multiple simulations (by calling
        muse::Simulation::finalizeSimulation(false, false))
    */
    virtual ~Simulation();
    
    /** \brief Interface method to enable derived classes to complete
        initialization of the simulator/kernel.

        <p>Once the simulation instance is created, it must be full
        initialized.  Notably, this includes initializing MPI (or
        other communication subsystems). Afterwards, important
        instance variables such as the SimulatorID are set.</p>

        <p>Accordingly, this method provides the necessary interface
        to permit derived classes to perform necessary initialization.
        This method is invoked from the
        muse::Simulation::initializeSimulation() right after this
        object has been created.</p>
        
        \note The derived class must always call the base class method
        to perform generic initialization associated with all the
        Simulator kernels.  Calling base class method is important and
        must not be skipped!
        
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
    virtual void initialize(int& argc, char* argv[], bool initMPI = true);

    /** \brief Finalize the Simulation

        This method will cause every Agent in the system to finalize,
        and will then delete the GVTManager and finalize the
        communicator.

        This method will also print statistics about the simulation.

        \param[in] stopMPI If this flag is true, then MPI is
        finalized.  Otherwise MPI is not finalized permitting another
        simulation run using current setup.

        \param[in] delCommMgr If this flag is true, then the
        communication manager is deleted.  Deletion of communication
        manager may be disabled by derived classes (if it is shared or
        needs to be used for other tasks).
    */
    virtual void finalize(bool stopMPI = true, bool delCommMgr = true);
    
    /** \brief Schedule the specified event
        
	Agents actually use this method to schedule events that are
	not local.

	\note Users should not be using this method. If used, will
        cause undefined behavior.  Use Agent::scheduleEvent method to
        avoid potential problems.
     
	\param e The event you wish to schedule.  This parameter
	cannot be NULL.  The event must have a valid receive time and
	receiver agentID filled-in.
        
	\return True if scheduling is successful.
        
	\see Event()
    */
    virtual bool scheduleEvent(Event* e);
        
    /** \brief Clean up old States and Events

        When this method is called, garbage collections for all the agents
        that are registered to this kernel take place.

        Please see Agent::collectGarbage for more details about how
        garbage is collected at the agent level.
       
        \see GVTManager
        \see Time
     */
    virtual void garbageCollect();

    /** Refactored utility method to parse command-line arguments
        pertinent to the kernel.

        This method is called just once from the initialize method to
        parse out the command-line arguments associated with the
        kernel.

	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.        
    */
    virtual void parseCommandLineArgs(int &argc, char* argv[]);

    /** Refactored utility method to report aggregate statistics at
        the end of simulation.

        This method was introduced to streamline the process of
        reporting to statistics while ensuring that the statistics do
        not get garbled during parallel simulation.  This method is
        invoked on all processes. On non-root (i.e., MPI rank > 0) the
        stats are generated in a string buffer and sent to the root
        kernel for reporting.  The root kernel obtains statistics from
        each kerel and reports it on the given output stream.

        \param[out] os The output stream to which statistics are to be
        written. By default statistics are reported to std::cout.
    */
    virtual void reportStatistics(std::ostream& os = std::cout);

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
    virtual void reportLocalStatistics(std::ostream& = std::cout) {}
    
    /** \brief Do the actual stats dump

        This method is called by the dumpStatsSignalHandler to
        actually do the work
     */
    virtual void dumpStats();

    /** \brief Convenience method to perform initialization/setup just
        before agents are initialized.

        This is a convenience method that derived classes can override
        to perform any additional initialization/setup operations
        prior to commencement of simulation.

		\note In multi threaded sims, this method is only called by the 
		manager thread as the commManager and gvtManager are shared between
		threads.
    */
    virtual void preStartInit();

    /** \brief Refactored method that initializes all the agents and
        creates initial state for them.

        The base class method iterates over each agent and performs
        the following operations:
        <ol>
        <li>Sets their LVT to startTime</li>
        <li>Invokes initialize() method on the agent</li>
        <li>Saves initial state of the agent by calling saveState method.</li>
        </ol>
        
        This is a convenience method that derived classes can override
        to perform any additional operations in conjunction with
        initialization of agents.
    */
    virtual void initAgents();


    /** \brief Refactored method to perform backoff to improve
        efficiency of reading messages from incoming MPI/network
        messages.

        This method is repeatedly invoked from the core simulation
        loop (in start() method) to process a batch of MPI messages.
        However, in simulations with few network messages, this call
        can be expensive as no messages may be read.  Consequently,
        this method adjusts the mpiMsgCheckThresh depending on number
        of events read.  The code below summarizes the logic:

        \code
        
        int numEvts = -1;
        if (--mpiMsgCheckCounter == 0) {
            numEvts = processMpiMsgs();
            if (numEvts == 0) {
                mpiMsgCheckThresh = std::min(10240, mpiMsgCheckThresh * 2);
            } else {
                mpiMsgCheckThresh = std::min(1, mpiMsgCheckThresh / 2);
            }
            mpiMsgCheckCounter = mpiMsgCheckThresh;
        }
        return numEvts;
        
        \endcode

        \note Derived classes typically do not overload this method.

        \return This method returns -1 if the call to processMpiMsgs()
        method was postponed.  Otherwise, it returns the number of MPI
        messages reported by processMpiMsgs() method.
    */
    virtual int checkProcessMpiMsgs();
    
    /** \brief Refactored method to process a block of incoming
        MPI/network messages.

        This method is repeatedly invoked from the core simulation
        loop from checkProcessMpiMsgs (which inturn is called from
        start() method) to process a maximum of maxMpiMsgThresh
        messages as suggested in the loop below (shown as pseudo
        code):

        \code

        int numEvts = maxMpiMsgThresh;
        while ((numEvts-- > 0) && (haveMpiMsgToProcess)) {
            // Process/schedule incoming event.
            // Let the GVT Manager forward any pending control
            // messages, if needed
            gvtManager->checkWaitingCtrlMsg();
        }
        
        \endcode

        This is a convenience method that derived classes can override
        to perform any additional operations in conjunction with
        message processing.

        \return This method returns the number of MPI messages that
        were received and processed.  If no messages were received,
        this method returns zero.
    */
    virtual int processMpiMsgs();

    /** \brief Refactored method to process the next set of events (if
        any) associated with 1 agent.

        This method is repeatedly invoked from the core simulation
        loop (in start() method) to process the next set of events for
        1 agent.  This is a convenience method that derived classes
        can override to perform any additional operations in
        conjunction with message processing.

        \return If event(s) were scheduled/processed this method
        returns true.  If no events were processed (either because of
        time-window restrictions or there were no events) then this
        method returns false.
    */
    virtual bool processNextEvent();
    
protected:
    /** All of the Agents in the Simulation
     *  
     *  This is shared between all instances of Simulation if multi threaded
     */
    AgentContainer allAgents;

    // Globally unique ID of the simulator -- MPI_Rank in this case
    SimulatorID myID;

    Time startTime, endTime;
    
    /**
     * local virtual time of this simulation kernel, unique to each thread
     * 
     * Note: There may be more than one associated kernel running on multple
     * threads on the same process with differing LGVT values.
     */
    Time LGVT;

    /** The name associated with the scheduler.

        This instance variable serves as a command-line argument value
        that is used to determine the actual scheduler to be used.
        The valid options are "default" and "hrm".  This value can be
        set via the implicit \c --scheduler command-line argument.
    */
    std::string schedulerName;
    
    /** Instance variable to hold a pointer to the Scheduler.  
	This instance variable holds pointer to an Scheduler object
	that is used to the next agent to process its next set of events.
    */
    Scheduler* scheduler;

    /** Instance variable to hold a pointer to the Communicator
	manager.  This instance variable holds pointer to an
	Communicator object that is used to send events to agents that
	reside on other Simulation kernels (nodes).
    */
    Communicator* commManager;

    /** Instance variable to hold a pointer to the GVT manager.  This
	instance variable holds pointer to an GVTManager object that
	is used to compute Global Virtual Time (GVT) in the
	simulation. GVT is necessary for automatic garbage collection
	and to terminate a parallel simulation.  This instance
	variable is initialized only after the simulation has been
	initialized.
    */
    GVTManager* gvtManager;

    /** Flag to indicate if all agents must save state at the end of
        each event processing cycle.

        This flag is set to false if only one MPI process is being
        used for simulation.  A user may force state saving using the
        --must-save-state command-line parameter.  If number of
        process is more than one then this flag is always true and
        cannot be overridden.
    */
    bool mustSaveState;
    
    /**  Used to control the rate at which GVT estimation is performed.

         This instance variable serves as a command-line argument
         place holder to determine the rate at wich GVT estimation is
         triggered.  This value indicates the number of event cycles
         (each cycle involves processing concurrent events by an
         agent) to be processed before a GVT estimation is triggered.
         This value can be set via the \c --gvt-delay command-line
         argument.
    */
    int gvtDelayRate;

    /// Number of Processes collaborating in the Simulation
    unsigned int numberOfProcesses;
    
    /** The listener to be used to report occurance of major phases
	during simulation.

	This instance variable is initialized to NULL in the
	constructor. It is set/reset by the setListener method in this
	class.  It is used by various methods in this class to report
	occurance of major phases during simulation.
    */
    SimulationListener* listener;

    /** The maximum number of consecutive MPI/network messages to
        process prior to processing a local event.

        This parmaeter provides a convenience way to balance the time
        spent between processing network messages (which may have
        critical anti-messages) versus processing locally scheduled
        events which maybe on the critical path.  Currently this value
        is a user-specified upper limit and it is not adapted.  The
        default value is 100 and it can be changed via
        --max-mpi-msg-thresh command-line argument.  This value is
        used in the following manner (shown as pseudo code) in the
        core simulation loop:

        \code

        while (gvt < endTime) {
            int numEvts = maxMpiMsgThresh;
            while ((numEvts-- > 0) && (haveMpiMsgToProcess)) {
                // Process/schedule incoming event.
            }
            // process pending event in scheduler.
        }

        \endcode
    */
    int maxMpiMsgThresh;

    /** Flag to indicate if events are to be shared between different
        processes.

        This flag indicates if events events are being directly shared
        between two differen threads on this process.  In such
        scenarios, two different reference counters (one for send
        thread and another for receive thread) are internally used to
        manage and recycle events.  The default value is \c false.  It
        is overridden only by derived multi-threading simulation
        classes via \c --use-shared-events flag. 
    */
    bool doShareEvents;
    
    // Debug-only logging purposes.
    DEBUG(std::ofstream*  logFile);
    DEBUG(std::streambuf* oldstream);
    
    /** The name of the type of simulator this is
  
        Example names include "default", "mpi-mt", or "mpi-mt-shm"   
     */
    static std::string simName;

    /// Keep track of if stats should be dumped this cycle
    bool doDumpStats;

    /** The average number of consecutive MPI messages read/processed
        each time the processMpiMsgs method was called.

        This stats object tracks the average number of consecutive MPI
        messages read/processed.  This statistic does not track the
        calls that returned zero messages.
    */
    Avg mpiMsgBatchSize;

    /** Counter to track the number of times processMpiMsgs method was
        called.

        This instance variabe tracks the number of times the
        processMpiMsgs method was called.
    */
    size_t processMpiMsgCalls;

    /** Counter to determine if MPI message check is to be
        performed.

        This counter is initialized to mpiMsgCheckThresh value.  In
        each call to checkProcessMpiMsgs method, this value is
        decremented. If it reaches zero, it is reset back to
        mpiMsgCheckThresh and the actual processMpiMsgs() method is
        called.
    */
    int mpiMsgCheckCounter = 1;

    /** Threshold to manage rate at which checks for MPI messages are
        performed.

        Checking for MPI messages via call to MPI_IProbe is an
        expensive operation.  Consequently, calls to
        Communicator::receiveEvent (which calls MPI_IProbe) needs to
        be carefully managed.  This value is is used to manage the
        rate in the following manner -- For each call to
        processMpiMsgs in which the first call to receive an event
        over MPI returns no events, this value is doubled (with max
        value of 10240); otherwise it is halved (with minimum value of
        1).
    */
    int mpiMsgCheckThresh = 1;

    /** Statistic to track the maximum value of mpiMsgCheckThresh
        before it is reset back to 1.

        This statistic object tracs the maximum value of
        mpiMsgCheckThresh before it is reset back to 1. After
        sufficient samples have been gathered, this value is useful to
        quickly reset mpiMsgCheckCounter back to a high value rather
        than slowly increasing.
    */
    Avg maxMpiMsgCheckThresh;

private:
    /** The undefined copy constructor.

        This Simulation class is defined to be a singleton.  There
        should not be multiple instances or copies of this class.
        Consequently the copy constructor for this class is
        intentinally declared to be private and is undefined.
    */
    Simulation(const Simulation &);

    /** The undefined assignment operator.

        This Simulation class is defined to be a singleton.  There
        should not be multiple instances or copies of this class.
        Consequently the assignment operator for this class is
        intentinally declared to be private and is undefined.
    */
    Simulation& operator=(const Simulation&);
    
    /** \brief Handler for sigaction USR1/USR2

        This method is called upon receipt of a USR1/USR2. Depending
        on the type of signal received, it will either set stats to be
        dumped on the next cycle (safe, USR1) or dump them immediately
        (unsafe, will probably crash the Simulation, USR2).

     */
    static void dumpStatsSignalHandler(int sigNum);

    /** Reference to the singleton instance of the derived Simulation
        object.

        This pointer is initialized to NULL. The correct simulation
        object is set when the static initialize method is invoked.
    */
    static Simulation* kernel;
};

END_NAMESPACE(muse);

#endif
