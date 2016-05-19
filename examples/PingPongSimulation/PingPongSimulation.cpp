/* 
 * File:   PingPongSimulation.cpp
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:05 PM
 */

#include <iostream>
#include <cstdlib>
#include "PingPongAgent.h"
#include "Simulation.h"
#include "DataTypes.h"

// Named constants to make code more readable
const int PING_KERNEL = 0;
const int PONG_KERNEL = 1;

/*
 * The main method that setups the simulation and runs a
 * simple ping-pong simulation with exactly 2 agents.
 */
int main(int argc, char** argv) {
    // First get simulation kernel instance to work with
    muse::Simulation* kernel = muse::Simulation::getSimulator();
    // Now lets initialize the kernel (and MPI)
    kernel->initialize(argc, argv);

    // now SPMD way of coding comes into play here!!!
    // NOTE:: here we register the agents (players) to different simulation
    //        kernels!!!!!
    muse::SimulatorID kernel_id = kernel->getSimulatorID();
    if (kernel_id == PING_KERNEL){
        PingPongAgent *pp_agent = new PingPongAgent(PING_KERNEL);
        kernel->registerAgent(pp_agent);
    } else {
        PingPongAgent *pp_agent = new PingPongAgent(PONG_KERNEL);
        kernel->registerAgent(pp_agent);
    }

    // we set the start and end time of the simulation here
    muse::Time start = 0, end = (argc >= 2) ? std::stod(argv[1]) : 10;
    kernel->setStartTime(start);
    kernel->setStopTime(end);

    // we finally start the ping pong simulation here!!
    kernel->start();

    // now we finalize the kernel to make sure it cleans up.
    kernel->finalize();

    // lets not forget to clean up the mez we make :-)
    // we don't have to worry about the kernel it takes care itself!
    return 0;
}

