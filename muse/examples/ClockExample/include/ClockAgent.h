/////////////////// Creating Your Agent ////////////////////
// Creating your own agent is very simple. There are a couple of
// key steps you must remember to follow. The first thing you must 
// do is inherit from the Agent() base class. This means that you MUST
// override THREE methods. The initialize(), executeTask(), and finalize() method
// are left to the client to implement. You actually don't have to override these methods,
// but if thats the case then what is the point of subclassing from the Agent base class. 
// Please refer to the documentation for ideas of what these methods are suppose to do in the
// simulation.
//
// The second thing to remeber is that every Agent has a State that is always modified. This
// means you need to subclass the State() base class. Please refer to the document below for
// more info.
//
// The last thing to remember is that every State of a given agent is modified, the way this is
// done in the simulation is via Events. For a given Agent, there should be an Event that can 
// modify its State. Please refer to the document below for more info.
//////////////////////////////////////////////////////////////

#include "../../include/Agent.h"

using namespace muse;

class ClockAgent : public muse::Agent {

public:
	
	//quick ctor define
	ClockAgent();

	//quick dtor define
	~ClockAgent();

	//Do all things to init the agent for the simulation here.
	void initialize();

	//When there is an event to be processed by an agent
	//muse will call this function and pass in the event(s) via the
	//EventContainer (Please refer to the documentation for more info on EventContainer)
	void executeTask(const muse::EventContainer & events);//end executeTask

	//Do all things to finalize the agent for the simulation here.
	void finalize();

private:
	//every agent must have a State to be modifed and archieved
	ClockState * c_state;
};