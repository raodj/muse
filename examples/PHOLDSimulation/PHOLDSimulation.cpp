/* 
   Author: Meseret Gebre

   This is a Synthetic Simulation done for benchmarking. P-HOLD simulation is 
   meant to mimic a typical load for a given simulation and can be scaled.
   Simply play with the parameters args = <X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>
   X = Number of agents per row
   Y = Number of agents per column
   N = Number of initial events each agent starts with
   Delay = The Delay time for the receive time, this will be max range for a random from [0,1]
   Max Nodes = The max number of nodes to run P-HOLD.
   Simulation endTime = The end time for the simulation.

   @note Please see PHOLDAgent.cpp for more detail :-)
*/

#include <iostream>
#include "PHOLDAgent.h"
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
    
    if (argc != 7){
        cout << "Correct USAGE: "<<argv[0] <<" <X> <Y> <N> <Delay> <Max Nodes> <Simulation endTime>"<<endl;
        cout << "  X     = Number of agents per row\n" <<
            "  Y     = Number of agents per column\n" <<
            "  N     = Number of initial events each agent starts with\n" <<
            "  Delay = The Delay time for the receive time, this will be max range for a random from [0,1]\n" <<
            "  Max Nodes = The max number of nodes to run P-HOLD.\n" << 
            "  Simulation endTime = The end time for the simulation." << endl;
        exit(1);
    }
    //first get simulation kernel instance to work with
    Simulation * kernel = Simulation::getSimulator();
    
    //now lets initialize the kernel
    kernel->initialize(argc,argv);

    //variables we need
    int x             = atoi(argv[1]);
    int y             = atoi(argv[2]);
    int n             = atoi(argv[3]);
    int delay         = atoi(argv[4]);
    int max_nodes     = atoi(argv[5]);
    int endTime       = atoi(argv[6]);
    int max_agents    = x*y;
    int agentsPerNode = max_agents/max_nodes;
    int rank          = kernel->getSimulatorID(); 

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
        State *phold_state = new State();
        id =  (max_agents/max_nodes)*rank + i;
        PHOLDAgent *phold_agent = new PHOLDAgent(id,phold_state,x,y,n,delay);
        // cout << "Rank: " << rank << " is servicing lp: " << id << endl;
        kernel->registerAgent(phold_agent);
    }//end for
    
    //we set the start and end time of the simulation here
    Time start=0, end=endTime;
    kernel->setStartTime(start);
    kernel->setStopTime(end);
    kernel->setGVTDelayRate(4000);
    //we finally start the ping pong simulation here!!
    kernel->start();
    
    //now we finalize the kernel to make sure it cleans up.
    kernel->finalize();
    
    return (0);
}

