/* 
 * File:   ClockExample.cpp
 * Author: gebremr
 *
 * Created on December 10, 2008, 9:44 PM
 */

#include <iostream>
#include "Simulation.h"
#include "ClockAgent.h"
#include "ClockState.h"

using namespace muse;
using namespace std;

/*
 * 
 */
int main() {

    cout << "Clock Example via MUSE API\n\n" << endl;
    //lets get an instance of the simulation kernel
    Simulation * kernel = Simulation::getSimulator();

    //Create our clock state
    ClockState * state =  new ClockState();
    AgentID id = 1;

    ClockAgent *cagent = new ClockAgent(id,state);

    //now we register our clock agent with the kernel
    kernel->registerAgent(cagent);

    //now we set our start and end times
    kernel->setStartTime(0) ;
    kernel->setStopTime(10);

    //finally start the simulation
    kernel->initialize();
    kernel->start()     ;
    kernel->finalize()  ;
    
    delete state;
    delete cagent;
    return (EXIT_SUCCESS);
}

