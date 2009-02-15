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

    /**The initialize method.
     * Once the simulation instance is returned. This method must be called to
     * fully init the simulation kernel. Things like the simulatorID is generated
     * in this method.
     *
     * NOTE :: if this method is not called and you start the simulation then
     *         the SimualtorID will equal -1u !!
     */
    void initialize();

    /**The initialize method.
     * Once the simulation instance is returned. This method must be called to
     * fully init the simulation kernel. Things like the simulatorID is generated
     * in this method.
     *
     * NOTE :: if this method is not called and you start the simulation then
     *         the SimualtorID will equal -1u !!
     *
     * @param argc, the number of arguments
     * @param argv, the arguments to pass into MPI.
     */
    void initialize(int argc, char* argv[]);

    /**The finalize method.
     * After calling the start method and the simualtion starts, this should be
     * the next method called. In this method clean up occurs. Running this method
     * will insure that all memory used by the Simulation kernel are properly disposed!
     *
     * NOTE :: if this method is called the SimualtorID will equal -1u !!
     */
    void finalize();
        
    /** The getSimulatorID method.
     * Use this method to obtain the id of the simulator you are working with. Typically 
     * in parallel simulation this will be the MPI_Rank. 
     *
     *@return Simulator* pointer to the id of the simulator
     *@see SimulatorID
     */
    SimulatorID getSimulatorID();
        
    /** The registerAgent method.
     * Once you design and create an agent for your simulation, use this method to register the agent to the 
     * simulator. For example a client could check the SimulatorID and with a switch statement register agents to 
     * different simulators on different processes.
     *
     *@param agent this is of type Agent. Agent passed in will not be modified.
     *@return AgentID* once registered an id is automatically generated and will be globally unique.
     *@see Agent()
     *@see AgentID 
     */
    bool registerAgent(Agent * agent);
        
    /** The getRegisteredAgents method.
     * When this method is invoked, it will return all agents that are registered to a given simulator.
     * Basically returns the container all simualtion objects hold.
     *
     *@return AgentContainer * a pointer to a container
     *@see AgentContainer
     */
    const AgentContainer& getRegisteredAgents();
        
    /** The getSimulator method.
     * The simulation class implaments a singleton pattern, then only way to access reference to the only object 
     * of type Simulation is via this method. Calling this method it will check if there is already an object created
     * if there is, the client will receive a pointer to that object, otherwise a Simulation object is created then 
     * the client will receive a pointer to that object. The client will need this object to get the ball rolling.
     * 
     * @return Simulation* the pointer to the Simulatin object.
     *
     */
    static Simulation* getSimulator();
        
    /** The scheduleEvent method.
     * Agents actually use this method to schedule events.
     * Users should not be using this method, when possible use the
     * Agent method to avoid potential problems.
     *
     * @param e a pointer to the event you wish to schedule
     * @return bool true if process is successful.
     */
    bool scheduleEvent( Event *e);

    /** The scheduleEvents method.
     * Agents actually use this method to schedule events.
     * Users should not be using this method, when possible use the
     * Agent method to avoid potential problems.
     *
     * @param e a pointer to the event you wish to schedule
     * @return bool true if process is successful.
     */
    //bool Simulation::scheduleEvents( EventContainer *events);

    /**The start method.
     * When this method is invoked the client should have all agents registered. The simulation will start.
     *
     */
    void start();
        
    /**The setStartTime method.
     * Sets the simulation start time. Keep in mind that the simulation does not have to start at time
     * Zero(0) each simulation object could start at different times. Warning, if you decided to start
     * at different times, it could cause rollbacks.
     *
     */
    void setStartTime(Time startTime);

    /** The stop method.
     * When this method is invoked the simulation will come to a big STOP. muse will go through and finalize all 
     * agents and clean up.
     *
     *@todo does this method make sense to store here.
     */
    void stop();
        
    /**The setStopTime method.
     * Sets the simulation stop time. 
     *
     */
    void setStopTime(Time stopTime);

    bool isAgentLocal(AgentID &);

    const Time& getTime();
    Time getLGVT() const;
    const Time& getStartTime();
    const Time& getEndTime();
	 
    
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
