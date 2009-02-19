/* 
 * File:   PingPongSimulation.cpp
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:05 PM
 */

#include <iostream>
#include "PingPongAgent.h"
#include "Simulation.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;
using namespace std;

#define PING_KERNEL 0
#define PONG_KERNEL 1

/*
 * 
 */
int main(int argc, char** argv) {
  //cout << "Ping Pong Simulation Example via MUSE API\n\n" << endl;
    //first get simulation kernel instance to work with
    Simulation * kernel = Simulation::getSimulator();
    
    //now lets initialize the kernel
    kernel->initialize(argc,argv);

    State *pingpong_state = new State();
    
    //now SPMD way of coding comes into play here!!!
    //NOTE:: here we register the agents (players) to different simulation
    //       kernels!!!!!
    SimulatorID kernel_id = kernel->getSimulatorID();
    //cout << "Kernel ID: " << kernel_id << endl;
    PingPongAgent *pp_agent = NULL;
    if (kernel_id == PING_KERNEL){
        AgentID my_id = 1;
        pp_agent = new PingPongAgent(my_id,pingpong_state);
        kernel->registerAgent(pp_agent);
    }else if (kernel_id == PONG_KERNEL){
        AgentID my_id = 2;
        pp_agent = new PingPongAgent(my_id,pingpong_state);
        kernel->registerAgent(pp_agent);
    }//end if-elseif

    //we set the start and end time of the simulation here
    Time start=0, end=20000; 
    kernel->setStartTime(start);
    kernel->setStopTime(end);

    //we finally start the ping pong simulation here!!
    kernel->start();

    //now we finalize the kernel to make sure it cleans up.
    kernel->finalize();

    //lets not forget to clean up the mez we make :-)
    //we don't have to worry about the kernel it takes care itself!
    return 0;
}

