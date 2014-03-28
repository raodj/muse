/* 
 * File:   ClockExample.cpp
 * Author: gebremr
 *
 * Created on December 10, 2008, 9:44 PM
 */

#include <iostream>
#include "Simulation.h"
#include "ClockAgent.h"

/*
 * 
 */
int main(int argc, char *argv[]) {
    std::cout << "Clock Example via MUSE API\n\n" << std::endl;
    //lets get an instance of the simulation kernel
    Simulation* const kernel = Simulation::getSimulator();
    kernel->initialize(argc, argv);
    
    //Create our clock state
    ClockAgent *cagent = new ClockAgent(0);

    //now we register our clock agent with the kernel
    kernel->registerAgent(cagent);

    // now we set our start and end times
    kernel->setStartTime(0) ;
    kernel->setStopTime(1000);

    // finally start the simulation
    kernel->start();
    kernel->finalize();
    
    return 0;
}

