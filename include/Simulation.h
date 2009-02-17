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
//
//---------------------------------------------------------------------------

#include "Agent.h"
#include "Scheduler.h"
#include "Event.h"
#include "State.h"
#include "DataTypes.h"
#include "Communicator.h"

/** The muse namespace.
 * Everything in the api is in the muse namespace.
 *
 */
BEGIN_NAMESPACE(muse); //begin namespace declaration

// Forward declaration to make compiler happy and fast
class GVTManager;

/** The Simulation Class.
 
    This is the heart of muse. All core operation that the engine
    handles are done from this class. This class implaments the
    Singleton pattern and should NOT be used as a superclass to derive
    from. Also the client should not try and create an instance using
    the constructor. use the getSimulator() method which will return a
    pointer to a Simulation object. Once you have all your agents and
    their events in place you can get the simulation going with the
    following three easy steps:

    <ol>

    <li> Get a simulation object via the getSimulator() method.</li>

    <li> Register all your agents by repeatedly invoking the
    register() method.</li>

    <li>Finally get the simulation stared via the start() method. To
    stop the simulation early just call the stop() method.</li>

    </ol>
    
    
    @note refer to each method documentation for more details on the
    features provided.
    
*/
class Simulation {
public:

    /** The initialize method.
	Once the simulation instance is returned. This method must be called to
	fully init the simulation kernel. Things like the simulatorID is generated
	in this method.
     
	NOTE :: if this method is not called and you start the simulation then
	the SimualtorID will equal -1u !!
     */
    void initialize();

    /** The initialize method.
	Once the simulation instance is returned. This method must be called to
	fully init the simulation kernel. Things like the simulatorID is generated
	in this method.
	
	NOTE :: if this method is not called and you start the simulation then
	the SimualtorID will equal -1u !!
     
	@param argc, the number of arguments
	@param argv, the arguments to pass into MPI.
     */
    void initialize(int argc, char* argv[]);

    /** The finalize method.
	After calling the start method and the simualtion starts, this should be
	the next method called. In this method clean up occurs. Running this method
	will insure that all memory used by the Simulation kernel are properly disposed!
	
	NOTE :: if this method is called the SimualtorID will equal -1u !!
     */
    void finalize();
        
    /** The getSimulatorID method.
	Use this method to obtain the id of the simulator you are working with. Typically 
	in parallel simulation this will be the MPI_Rank. 
     
	@return  the id of the simulation kernel
	@see SimulatorID
     */
    inline SimulatorID getSimulatorID() const { return myID;}
        
    /** The registerAgent method.
	Once you design and create an agent for your simulation, use this method to register the agent to the 
	simulator. For example a client could check the SimulatorID and with a switch statement register agents to 
	different simulators on different processes.See examples.

	IMPORTANT - Once regiestered agents will be handled by MUSE. Deleting of agent will be handled by MUSE. Please only allocate agent on the heap.
     
	@param agent pointer, this is of type Agent.
	@return bool, True if the agent was register correctly.
	@see Agent()
	@see AgentID 
     */
    bool registerAgent(Agent * agent);
        
    /** The getRegisteredAgents method.
	When this method is invoked, it will return all agents that are registered to a given simulation kernel.
	
	@return reference to the AgentContainer. 
	@see AgentContainer
     */
    const AgentContainer& getRegisteredAgents();
        
    /** The getSimulator method.
	The simulation class implements a singleton pattern. Call this method to get a pointer to the simulation kernel.
	
	@return the pointer to the Simulatin object.
	@see Simulation()
     */
    static Simulation* getSimulator();
        
    /** The scheduleEvent method.
	Agents actually use this method to schedule events.
	Users should not be using this method, when possible use the
	Agent method to avoid potential problems.
     
	@param pointer to the event you wish to schedule.
	@return bool True if scheduling is successful.
	@see Event()
     */
    bool scheduleEvent( Event *e);

    
    /** The start method.
	When this method is invoked the client should have all agents registered.
	Also a flavor of the initialize method should be called.Lastly, you should set the start and end time for the simuatlion.
	The simulation will start.
     */
    void start();
        
    /** The setStartTime method.
	Sets the simulation start time. Keep in mind that the simulation does not have to start at time
	Zero(0) each simulation object could start at different times. Warning, if you decided to start
	at different times, it could cause rollbacks.

	@param the start time.
	@see Time
     */
    void setStartTime(Time startTime);

    /** The stop method.
	When this method is invoked the simulation will come to a big STOP. muse will go through and finalize all 
	agents and clean up.

	@todo implement this. Currently does nothing.
	@todo does this method make sense to store here.
     */
    void stop();
        
    /** The setStopTime method.
	Sets the simulation stop time. 

	@param the stop time.
	@see Time
     */
    void setStopTime(Time stopTime);

    /** The isAgentLocal method.
	Used to check if a given AgentID is local to this kernel.

	@param reference to the Agent's id
	@return bool, True if the AgentID is registered to this kernel.
    */
    bool isAgentLocal(AgentID &);

    /** The getTime method.
	This is the time of this simualtion kernel.
    */
    inline const Time& getTime() const { return LGVT; }

    
    const Time& getLGVT() const;

    /** The getStartTime method.
	@return the start time of this simulation kernel.
	@see Time
     */
    inline const Time& getStartTime() const { return startTime;}

    /** The getEndTime method.
	@return the end time of this simulation kernel.
	@see Time
     */
    inline const Time& getEndTime() const { return endTime; }
	 
    
private:

    //the kernel singleton instance
    ////the ctor method, must be private (singleton pattern)
    Simulation();
    Simulation(const Simulation &);
    Simulation& operator=(const Simulation&);
    ~Simulation();

    //used to contain all agents registered to this simulator
    AgentContainer allAgents;

    //usually the MPI_Rank, otherwise a globally unique id for the simulator.
    SimulatorID myID;

    Time LGVT, startTime, endTime;
    Scheduler scheduler;
    //handles all communication on the wire. (MPI)
    Communicator commManager;

    /** Instance variable to hold a pointer to the GVT manager.
        
        This instance variable holds pointer to an GVTManager object
        that is used to compute Global Virtual Time (GVT) in the
        simulation. GVT is necessary for automatic garbage collection
        and to terminate a parallel simulation.  This instance variable
        is initialized only after the simulation has been initialized.
    */
    GVTManager *gvtManager;
};
 
END_NAMESPACE(muse); //end namespace declaration

#endif
