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

#include <csignal>

BEGIN_NAMESPACE(muse);

// Forward declaration to make compiler happy and fast
class GVTManager;
class Scheduler;
class Communicator;
class SimulationListener;

/** The Simulation Class.
 
    This is the heart of muse. All core operation that the engine
    handles are done from this class. This class implaments the
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
    */
    void initialize(int& argc, char* argv[]) throw (std::exception);

    /** \brief Finalize the Simulation

        This method will cause every Agent in the system to finalize,
        and will then delete the GVTManager and finalize the
        communicator.

        This method will also print statistics about the simulation.
    */
    void finalize();
        
    /** \brief Get the Simulator ID
     
	\return The ID of the simulation kernel
        
	\see SimulatorID
    */
    inline SimulatorID getSimulatorID() const { return myID; }

    /** \brief Get the total number of processes

        \return The total number of processes collaborating in this
        simulation
     */
    inline unsigned int getNumberOfProcesses() const { return numberOfProcesses;}
    
    /** \brief Register the given Agent to the Simulation

        Once you design and create an agent for your simulation, use
	this method to register the agent to the simulator. For
	example a client could check the SimulatorID and with a switch
	statement register agents to different simulators on different
	processes. See examples.

	\note Once regiestered agents will be handled by
	MUSE. Deleting of agent will be handled by MUSE. Please only
	allocate agent on the heap.
     
	\param agent pointer, this is of type Agent.
	\return True if the agent was register correctly.
	\see Agent()
	\see AgentID 
    */
    bool registerAgent(Agent* agent);
        
    /** \brief Get all Agents registered to the simulation
	
	\return An AgentContainer with all registered agents
	\see AgentContainer
    */
    inline const AgentContainer& getRegisteredAgents() const { return allAgents; }
        
    /** \brief Get the only Simulation

        The simulation class implements a singleton pattern. Call this
	method to get a pointer to the simulation kernel.
	
	\return A pointer to the one-and-only Simulation
    */
    static Simulation* getSimulator();

    
    /** \brief Start the Simulation

        This method should be called after all of the appropriate
        initialization has been completed.  Upon doing so, the
        Simulation will start.
    */
    void start();
        
    /** \brief Sets the simulation start time.

        Keep in mind that the simulation does not have to start at
	time Zero (0) -- each simulation object could start at
	different times. Warning, if you decided to start at different
	times, it could cause rollbacks.

	\param sTime The start time
	\see Time
    */
    inline void setStartTime(Time sTime) { startTime = sTime; }

    /** \brief Stop the simulation

        Currently unimplemented.

        This method stops the Simulation.  There should be no need to
        use this method, use setStopTime() instead.
        
	\todo Implement this. Currently does nothing.
    */
    void stop();
        
    /** \brief Sets the simulation stop time. 

	\param eTime The stop time.
	\see Time
    */
    inline void setStopTime(Time eTime) { endTime = eTime;}

    /** \brief Set the GVT Delay Rate
        
	Different Simulation models perform better with different
	rates of GVTEstimation.  If you have a large number of agents
	its better to make this a smaller number between (1-50)
	otherwise it would make your simulation faster to minimize
	garbage collection

	\note The default is rate is 100
        
	\param rate The rate at which to delay GVT estimation (rate
	representing timesteps)
	
     */
    inline void setGVTDelayRate(int rate) { gvtDelayRate = rate;}
    
    /** \brief Determine if the givent Agent is local 

	This method will check the registration tables to determine if
	the Agent that has the given ID is local or not

	\param[in] id ID of the agent to check
	\return True if the AgentID is registered to this kernel.
    */
    bool isAgentLocal(AgentID id);
    
    /** \brief Get the start-time of the Simulation

        \return the start time of this simulation
	\see Time
    */
    inline Time getStartTime() const { return startTime; }

    /** \brief Get the end-time of the Simulation
        
	\return the end time of this simulation
	\see Time
    */
    inline Time getStopTime() const { return endTime; }

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
    void setListener(SimulationListener* listener);
    
protected:
    /** \brief Schedule the specified event
        
	Agents actually use this method to schedule events that are
	not local.

	\note Users should not be using this method. If used, will
        cause undefined behavior.  Use Agent::scheduleEvent method to
        avoid potential problems.
     
	\param e The event you wish to schedule.
	\return True if scheduling is successful.
	\see Event()
    */
    bool scheduleEvent(Event* e);
    
    /** \brief Get the Global Virtual Time
        
        \return The GVT
        \see Time
    */
    Time getGVT() const;

    /** \brief Get the LGVT
        
        The LGVT is the next time that will become the LVT.

	\return The LGVT
	\see Time
     */
    Time getLGVT() const;

    /** \brief Update the specified Agent's key to the specified time

        \see Scheduler#updateKey
       
     */
    // void updateKey(void* pointer, Time uTime);
    
    /** \brief Clean up old States and Events

        When this method is called, garbage collections for all the agents
        that are registered to this kernel take place.

        Please see Agent::collectGarbage for more details about how
        garbage is collected at the agent level.
       
        \see GVTManager
        \see Time
     */
    void garbageCollect();

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
    void parseCommandLineArgs(int &argc, char* argv[]);

    /** Refactored utility method to report aggregate statistics at
        the end of simulation.

        This method was introduced to streamline the process of
        reporting to statistics while ensuring that the statistics do
        not get garbled during parallel simulation.  This method is
        invoked on all processes. On non-root (i.e., MPI rank > 0) the
        stats are generated in a string buffer and sent to the root
        kernel for reporting.  The root kernel obtains statistics from
        each kerel and reports in rank order on the given output
        stream.

        \param[out] os The output stream to which statistics are to be
        written. By default statistics are reported to std::cout.
    */
    void reportStatistics(std::ostream& os = std::cout);
    
private:
    // Constructors and Destructor -- Private to enforce Singleton
    // pattern
    Simulation();
    Simulation(const Simulation &);
    Simulation& operator=(const Simulation&);
    ~Simulation();

    /// All of the Agents in the Simulation
    AgentContainer allAgents;

    // Globally unique ID of the simulator -- MPI_Rank in this case
    SimulatorID myID;

    Time LGVT, startTime, endTime;

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
    
    // Debug-only logging purposes.
    DEBUG(std::ofstream*  logFile);
    DEBUG(std::streambuf* oldstream);

    /** \brief Handler for sigaction USR1/USR2

        This method is called upon receipt of a USR1/USR2. Depending
        on the type of signal received, it will either set stats to be
        dumped on the next cycle (safe, USR1) or dump them immediately
        (unsafe, will probably crash the Simulation, USR2).

     */
    static void dumpStatsSignalHandler(int sigNum);

    /** \brief Do the actual stats dump

        This method is called by the dumpStatsSignalHandler to actuall
        do the work
     */
    void dumpStats();

    /// Keep track of if stats should be dumped this cycle
    bool doDumpStats;
};

END_NAMESPACE(muse);

#endif
