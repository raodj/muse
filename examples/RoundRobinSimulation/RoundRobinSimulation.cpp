/* 
  Author: Meseret Gebre

  This is a Synthetic Simulation done for benchmarking. This is seeing how well
  MUSE can handle low parallelism. The Round Robin simply passes events around. 
  Starting with agent N , creates an event and sends it to agent N+1. This continues 
  in a Round Robin fashion. We test for scability and efficiency.
 */

#include <vector>
#include <stdlib.h>

#include <iostream>
#include "RoundRobinAgent.h"
#include "Simulation.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;
using namespace std;

/*
 */
int main(int argc, char** argv) {
    
    if (argc != 4){
        cout << "Correct USAGE: "<<argv[0] <<" <num agents> <num nodes> <sim end time >"<<endl;
        exit(0);
    }

    //first get simulation kernel instance to work with
    Simulation * kernel = Simulation::getSimulator();
    
    //now lets initialize the kernel
    kernel->initialize(argc,argv);

	//variables we need
    int max_agents    = atoi(argv[1]);
    int max_nodes     = atoi(argv[2]);
    int endTime       = atoi(argv[3]);
    int agentsPerNode = max_agents/max_nodes;
    int rank = kernel->getSimulatorID(); 

    
    ASSERT(max_agents >= max_nodes);
    //check to make sure agents fit into nodes
    //if uneven then last node carries the extra
    //agents.
    if ( rank == (max_nodes-1) && (max_agents % max_nodes) > 0 ){
        //means we have to add the extra agents to the last kernel
        agentsPerNode = (max_agents/max_nodes) + (max_agents % max_nodes);
       
    }

    //cout << "rank: "<<rank << " has APN: "<< agentsPerNode <<endl;
    
    AgentID id = -1u;
    for (AgentID i= 0;i < agentsPerNode; i++){
        State *rb_state = new State();
        id =  (max_agents/max_nodes)*rank + i;
        //cout << "Round Robin Agent id: " << id <<endl;
        RoundRobinAgent *rb_agent = new RoundRobinAgent(id,rb_state,max_agents);
        kernel->registerAgent(rb_agent);
    }//end for
    
    //kernel->registerAgent(agents[kernel->getSimulatorID()]);
    //we set the start and end time of the simulation here
    Time start=0, end=endTime;
    kernel->setStartTime(start);
    kernel->setStopTime(end);
    
    //we finally start the ping pong simulation here!!
    kernel->start();
    
    //now we finalize the kernel to make sure it cleans up.
    kernel->finalize();
    
    return (0);
}

