#ifndef _MUSE_COMMUNICATOR_H_
#define _MUSE_COMMUNICATOR_H_

#include "DataTypes.h"
#include <map>

//these are the tag types
#define AGENT_LIST      0
#define EVENT           1

//these are the source types
#define ROOT_KERNEL     0


BEGIN_NAMESPACE(muse);
class Communicator {

//class Event;

public:
        /** The ctor.
         */
	Communicator();

         /** The dtor.
          */
	~Communicator();

         /** The sendEvent method.
          *
          * @param Event pointer, this is the event to be sent across the wire.
          * @param int eventSize, simply use the sizeof() method of the event class
          *        you are sending on the wire.
          *        USAGE EXAMPLE:: using the clock example!
          *              ClockEvent *e = new ClockEvent();
          *              sendEvent(e, sizeof(e));
          *                 .....
          *              free(e); //please remember to free your memory :-)
          * @see Event
          *
          */
	void sendEvent(Event *, int &);

        /** The recvEvent method.
          *
          * NOTE:: Will return a NULL if there is no Event to receive!!!
          * @return Event pointer, this is the event to be received from the wire.
          * @see Event
          */
	Event* receiveEvent();

        /** The initialize method
         * this is used to sync all communicator and get the agentMap
         * populated.
         */
	SimulatorID initialize();

        /**The registerAgents method.
         * This is used to sync the AgentMapping in the Communicator class.
         * Before you can send any events across the wire (MPI), use this to
         * get your agents and other kernel's agents mapped correctly.
         *
         * NOTE:: please call the initialize method before calling this method.
         *
         *@param AgentContainer reference, this is the list of agents the kernel containts
         */
        void registerAgents(AgentContainer &);

        /** The finialize method.
         *
         * Use this to clean up after yourself.
         *
         */
	void finialize();

private:

    /** This is used to map the locations of all agents in the simulation.
     *  When simulation starts, all simulation kernels, will perform a bcast and
     *  send all agents that they have. Using the agentID as the key, the communicator
     *  will be able to know the simulator kernel's ID.
     *
     * @see AgentID
     * @see SimulatorID
     */
    map<AgentID, SimulatorID> agentMap;
};

END_NAMESPACE(muse);

#endif
