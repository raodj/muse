#ifndef _MUSE_SIMUALTION_H_
#define _MUSE_SIMUALTION_H_

//---------------------------------------------------------------------------
// USE AS IS, muse will not be responsible if anything breaks, while using muse.
// Authors: Meseret Gebre       gebremr@muohio.edu
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
BEGIN_NAMESPACE(muse) //begin namespace declaration

        
/** The Simulation Class.
 *
 * This is the heart of muse. All core operation that the engine handles are done from this class. This class
 * implaments the Singleton pattern and should NOT be used as a superclass to derive from. Also the client should
 * not try and create an instance using the constructor. use the getSimulator() method which will return a pointer to 
 * a Simulation object. Once you have all your agents and their events in place you can get the simulation going with 
 * three easy steps. 1st - get a simulation object via the getSimulator() method. 2nd - register all your agents. 3rd - finally
 * get the simulation stared via the start() method. To stop the simulation early just call the stop() method.
 *
 * Note: refer to each method documentation for more details on the features provided.
 *
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
	void setStartTime(const Time & startTime);

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
	void setStopTime(const Time & stopTime);


        const Time& getTime();
        const Time& getStartTime();
        const Time& getEndTime();
	 
protected:
    //the ctor method, must be private (singleton pattern)
    Simulation();
    Simulation(const Simulation &);
    Simulation& operator=(const Simulation&);
    ~Simulation();
private:

    //the kernel singleton instance
    //

    //used to contain all agents registered to this simulator
    AgentContainer allAgents;

    //usually the MPI_Rank, otherwise a globally unique id for the simulator.
    SimulatorID myID;

    Time LGVT, startTime, endTime;
    Scheduler scheduler;
    //handles all communication on the wire. (MPI)
    Communicator commManager;

};
 
END_NAMESPACE(muse)//end namespace declaration

#endif
