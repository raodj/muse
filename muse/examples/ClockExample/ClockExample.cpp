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
#include "../../include/Time.h"

using namespace muse; //everything is in this namespace, make sure to use this namespace

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


