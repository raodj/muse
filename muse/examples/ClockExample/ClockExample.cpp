/////////////////////////////////
// Author: Meseret Gebre     meseret.gebre@gmail.com
//
// This is a simple example that simulates a clock and all it does is 
// print out a notice every hours that passes. Hours in terms of simulation time
// so an hour could mean anything (user defined). The purpose of this example is to 
// show the basic usage of muse. Note, all classes are defined in one file because 
// this is a trivial example. 
/////////////////////////////////
#include <iostream>
#include "../../include/Simulation.h"
#include "../../include/Agent.h"
#include "../../include/Event.h"
#include "../../include/State.h"
#include "../../include/Time.h"

using namespace muse; //everything is in this namespace, make sure to use this namespace


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
class ClockAgent : public muse::Agent {

public:
	
	//quick ctor define
	ClockAgent(){
		this->time_zero = new Time(0);
		this->c_state = new ClockState(time_zero);
	}

	//quick dtor define
	~ClockAgent(){}

	//Do all things to init the agent for the simulation here.
	void initialize(){
		//For our example here we'll simply print that we are
		//initializing the agent
		std::cout << "ClockAgent has initialized!!" << std::endl;
	}

	//When there is an event to be processed by an agent
	//muse will call this function and pass in the event(s) via the
	//EventContainer (Please refer to the documentation for more info on EventContainer)
	void executeTask(const muse::EventContainer & events){
		//For our example, when we get an event to process
		//we'll simply print out that an hour of simulation time has elasped 
		//and then schedule an event to ourself to be sent an hour later.
		
	}

	//Do all things to finalize the agent for the simulation here.
	void finalize(){
		//For our example we will clean up all memory allocations and 
		//print that we are finalizing our agent.
		//delete(c_state); //despose our clock state
		//delete(time_zero); //depose our time object
		std::cout << "ClockAgent has finalized!!" << std::endl;
	}

private:
	//every agent must have a State to be modifed and archieved
	Time * time_zero;
	ClockState * c_state;
};

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

/////////////////// Creating Your Event ////////////////////
// Events and States have an unspoken relationship. I say this because 
// events will know how to modify the state variables and hence change
// an agent's state. There is nothing to override from the base class.
// Just add the need info to change the agents state.
///////////////////////////////////////////////////////////
class ClockEvent : public muse::Event {

	//For Our example the arrival of the event itself is the signal
	//we need to change the ClockState. Hence, we didn't even 
	// need to create a subclass of Event, but for clearity reasons,
	// I have choosen to do so.
};


//Your Simulation should follow a SPMD (Single Program Multiple Data) 
// http://en.wikipedia.org/wiki/SPMD is the wiki article if you don't know what SPMD is.
int main(){

	//////////// STEP 1 /////////////////////
	// First thing that you must do is get the Simulation object. 
	// If you have N machine to run the your simulation, then you will be provided
	// with N simultion, meaning you can register certian agents to certain machines.
	// Remember that the return type is a Simulation object that is const. You should 
	// not to modify this in any form. 
	// refer to the Documentation for the Simulation class for method specific docs!
	/////////////////////////////////////////
	 Simulation * my_simulator = muse::Simulation::getSimulator();

	//////////// STEP 2 /////////////////////
	// Second step is the registeration of your Agents. 
	// You have N machine, now tell muse where each Agent should run.
	// This is where the SPMD logic comes to play. Each Simulation object 
	// has a globally unique ID (refer to the Docs for more info). You can 
	// use this info (IDs are Zero (0) based) to tell muse where to deploy your
	// agents.
	//
	// For this example we only have one agent (ClockAgent) and this agent will 
	// be deployed on the first machine.
	/////////////////////////////////////////
	ClockAgent * c_agent = new ClockAgent(); // <-- create our agent here

	switch( my_simulator->getSimulatorID() ){
	case 0: //this means we are dealing with the simulation object on the 1st machine
		my_simulator->registerAgent(c_agent); // <-- easy registeration process.
		break;
	case 1://this means we are dealing with the simulation object on the 2nd machine and etc...
	default:
		//do nothing here
		break;
	};

	//////////// STEP 3 /////////////////////
	// The last step is to set the Simulator start Time (lets muse know when to start) ans stop Time.
	// Finally pressing the start button. Note, the simulation does not have to start at time Zero (0). 
	/////////////////////////////////////////
	muse::Time start_time(0); //this creates a time representing when the simulation should start
	muse::Time stop_time(10000); //stop simulation at Time 10 thousand.  
	my_simulator->start();
	//@ this point the simulation has ended and muse has finalized and cleaned up, now its our
	//time to clean up, remember memory leaks are no good :-)
	delete(c_agent);

}//end main


