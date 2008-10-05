#ifndef _MUSE_COMMUNICATOR_H_
#define _MUSE_COMMUNICATOR_H_

#include "Simulation.h"

namespace muse {
class Communicator {

class Event;

public:
	Communicator();
	~Communicator();

	void sendEvent(Event *, SimulatorID *);

	void initialize();

	void sendMessage(char *, SimulatorID *);

	void sendMessageToAll(char *); //not sure what this would be usefull for??

	
};
}

#endif
