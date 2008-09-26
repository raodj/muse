#ifndef _MUSE_SIMUALTION_H_
#define _MUSE_SIMUALTION_H_

//---------------------------------------------------------------------------
// USE AS IS, muse will not be responsible if anything breaks, while using muse.
// Authors: Meseret Gebre       gebremr@muohio.edu
//
//---------------------------------------------------------------------------


#include <set>
#include "../../include/Agent.h"
using std::set;

/** The muse namespace.
 * Everything in the api is in the muse namespace.
 *
 */
namespace muse { //begin namespace declaration 

        /** The SimulatorID struct.
         * This is used to store the id of a simulator(Singleton). muse uses this SimulatorID struct to make sure the client
         * know exactly what the param of a method is asking for. For more details check out the method getSimulatorID(). 
         * @see getSimulatorID()
         *
         */
	typedef unsigned int SimulatorID;

        /** The AgentContainer.
         * Holds a collection of agents, this is basically a set of Agents. Always 
         * use the AgentContainer type, because the underlying implamentation could change.
         *
         */
	typedef set<Agent> AgentContainer;

        
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
        /** The getSimulatorID method.
         * Use this method to obtain the id of the simulator you are working with. Typically 
         * in parallel simulation this will be the MPI_Rank. 
         *
         *@return Simulator* pointer to the id of the simulator
         *@see SimulatorID
         */
	const SimulatorID getSimulatorID();
        
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
	 const AgentID* registerAgent( const Agent * agent) const;
        
        /** The getRegisteredAgents method.
         * When this method is invoked, it will return all agents that are registered to a given simulator.
         * Basically returns the container all simualtion objects hold.
         *
         *@return AgentContainer * a pointer to a container
         *@see AgentContainer
         */
	const AgentContainer * getRegisteredAgents();
        
        /** The getSimulator method.
         * The simulation class implaments a singleton pattern, then only way to access reference to the only object 
         * of type Simulation is via this method. Calling this method it will check if there is already an object created
         * if there is, the client will receive a pointer to that object, otherwise a Simulation object is created then 
         * the client will receive a pointer to that object. The client will need this object to get the ball rolling.
         * 
         *
         *@return Simulation* the pointer to the Simulatin object.
         *
         */
        static Simulation* getSimulator();
        
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

        /**The dtor method.
         * Cleans up all mess created by muse.
         *
         */
	~Simulation();

private:
        //the ctor method, must be private (singleton pattern)
	Simulation();
        //used to contain all agents registered to this simulator
	AgentContainer allAgents;
        //usually the MPI_Rank, otherwise a globally unique id for the simulator. 
	SimulatorID myID;
};

}//end namespace declaration

#endif