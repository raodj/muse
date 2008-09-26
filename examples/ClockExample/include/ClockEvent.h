/////////////////// Creating Your Event ////////////////////
// Events and States have an unspoken relationship. I say this because 
// events will know how to modify the state variables and hence change
// an agent's state. There is nothing to override from the base class.
// Just add the need info to change the agents state.
///////////////////////////////////////////////////////////

#include "../../include/Event.h"
using namespace muse; //everything is in this namespace, make sure to use this namespace

class ClockEvent : public muse::Event {

	//For Our example the arrival of the event itself is the signal
	//we need to change the ClockState. Hence, we didn't even 
	// need to create a subclass of Event, but for clearity reasons,
	// I have choosen to do so.
};