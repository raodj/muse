/////////////////// Creating Your State ////////////////////
// The state of Agent should contain all data that is modifiable by the  
// agents event. In other word when an Agent receives an event to process
// typically the event processing in return cause changes to the Agent's state.
// Please refer to the documentation for more detail. For muse to handle states
// correctly it must know how to duplicate the agents state. Since the client 
// can added unlimited amount of data in the state, there is no way for muse to know
// exactly how to duplicate a particular state. Hence, the getClone() method *MUST*
// be overridden.
///////////////////////////////////////////////////////////


#include "../../include/State.h"
using namespace muse; //everything is in this namespace, make sure to use this namespace

class ClockState : public muse::State {
	
	
public:

	//the ctor simply initalizes the ClockState to the time in the param
	ClockState(Time * time): t(time){
		
	}

	//good practice is to create getters and setters instead of public
	// variables!
	void setClockTime(Time * time){
		t->time = time->time;
	}//end setClockTime

	Time * getClockTime(){
		return this->t;
	}//end getClockTime

	//the dtor cleans up the time pointer
	~ClockState(){
		delete(t);
	}

	//This method is very important and should be implemented with efficiency,
	//remember muse will call this method many times and should be efficient in every way possible.
	ClockState * getClone(){
		//here we will do a deep clone

		//Note this is one way to create the deep clone,
		//but this is not a efficient way to return a cloned
		//state object because this requires a call to ctor and 
		// when you return the object a new object is actually created
		// and this object calls the dtor on itself.
		/////ClockState * cloned_state = new ClockState();
		/////cloned_state->setClockTime(this->getClockTime());
		/////return cloned_state;
		

		//the better way is to just do this
		//this way you avoid the calls to ctor[constructor] and dtor[destructor]
		//some compiler would recognize this and not even create the new object,
		// just returns the values.
		return new ClockState(this->getClockTime());
	}
private:
	// for our example the only thing that is modified is 
	// the Clock's time, which is always updated via ClockEvents.
	Time * t; //set the initial time to zero (0).

};
