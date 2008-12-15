#ifndef _MUSE_COMMUNICATOR_H_
#define _MUSE_COMMUNICATOR_H_

#include "DataTypes.h"
#include <map>

BEGIN_NAMESPACE(muse);
class Communicator {

class Event;

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
          * @see Event
          */
	void sendEvent(Event *);

        /** The recvEvent method.
          *
          * @return Event pointer, this is the event to be received from the wire.
          * @see Event
          */
	Event * recvEvent();

        /** The initialize method
         * this is used to sync all communicator and get the agentMap
         * populated.
         */
	void initialize();

	void sendMessage(char *);

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
