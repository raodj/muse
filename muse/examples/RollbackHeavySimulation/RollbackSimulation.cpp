/* 
 * File:   RollbackSimulation.cpp
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:05 PM
 */

#include <vector>
#include <iostream>
#include <stdlib.h>
#include "arg_parser.h"
#include "RollbackAgent.h"
#include "Simulation.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;
using namespace std;


int max_agents;
int max_nodes;
int end_time;

//let make the arg_record
arg_parser::arg_record arg_list[] = {
    { "-agents","The number of agents in the simulation.", &max_agents, arg_parser::INTEGER },
    { "-nodes","The max numbers of nodes used for this simulation.", &max_nodes, arg_parser::INTEGER },
    { "-end","The end time for the simulation.", &end_time, arg_parser::INTEGER },
    
    { NULL, NULL }
};


/*
 */
int main(int argc, char** argv) {
    
    // if (argc != 4){
    //    cout << "Correct USAGE: "<<argv[0] <<" <num agents> <num nodes> <sim end time >"<<endl;
    //    exit(0);
    //}
    //default values for parameters
    
    max_agents  = 2;
    max_nodes   = 2;
    end_time    = 10;
    arg_parser ap( arg_list );
    ap.check_args( argc, argv ,true);

   
  //cout << "Ping Pong Simulation Example via MUSE API\n\n" << endl;
    //first get simulation kernel instance to work with
    Simulation * kernel = Simulation::getSimulator();
    
    //now lets initialize the kernel
    kernel->initialize(argc,argv);

    //variables we need
    //int max_agents    = atoi(argv[1]);
    ///int max_nodes     = atoi(argv[2]);
    //int end_time       = atoi(argv[3]);
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
        State *rb_state = new State();
        id =(max_agents/max_nodes) *kernel->getSimulatorID() + i;
        //cout << "i: " << id <<endl;
        RollbackAgent *rb_agent = new RollbackAgent(id,rb_state,max_agents);
        kernel->registerAgent(rb_agent);
    }//end for
    
    //kernel->registerAgent(agents[kernel->getSimulatorID()]);
    //we set the start and end time of the simulation here
    Time start=0, end=end_time;
    kernel->setStartTime(start);
    kernel->setStopTime(end);

    //we finally start the ping pong simulation here!!
    kernel->start();

    //now we finalize the kernel to make sure it cleans up.
    kernel->finalize();

    //lets not forget to clean up the mez we make :-)
    //we don't have to worry about the kernel it takes care itself
    // for(int i = 0; i < agents.size(); i++){
    //     if (i != kernel->getSimulatorID())
    //         delete agents[i];
    // }
    return (0);
}

