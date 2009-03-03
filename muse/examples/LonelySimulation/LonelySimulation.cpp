/* 
  Author: Meseret Gebre

  This is a Synthetic Simulation done for benchmarking. This is seeing how well
  MUSE can handle high parallelism. The Lonely Agent simply passes events to itself. 
  Starting with agent N , creates an event and sends it to agent N. This continues 
  until end time, hence the name Lonely Agent. We test for scability and efficiency.
 */

#include <vector>


#include <iostream>
#include "LonelyAgent.h"
#include "Simulation.h"
#include "State.h"
#include "DataTypes.h"
#include <math.h>
#include <cstdlib>

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
    
    AgentID id = -1u;
    for (AgentID i= 0;i < agentsPerNode; i++){
        State *rb_state = new State();
        id =  (max_agents/max_nodes)*rank + i;
        //cout << "Round Robin Agent id: " << id <<endl;
        LonelyAgent *rb_agent = new LonelyAgent(id,rb_state);
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

