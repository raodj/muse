#ifndef ROLLBACK_SIMULATION_CPP
#define ROLLBACK_SIMULATION_CPP

/* 
 * File:   RollbackSimulation.cpp
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:05 PM
 */

#include <vector>
#include <iostream>
#include <stdlib.h>
#include "ArgParser.h"
#include "RollbackAgent.h"
#include "Simulation.h"
#include "State.h"
#include "DataTypes.h"

// Setup default values for command-line arguments.
int max_agents = 2;   // Two agents total
int max_nodes  = 1;   // One node by default (no rollbacks in this case!)
int end_time   = 10;  // Simulation end time

// Make the arg_record
ArgParser::ArgRecord arg_list[] = {
    { "--agents", "The number of agents in the simulation.",
      &max_agents, ArgParser::INTEGER },
    { "--nodes","The number of compute-nodes being used for this simulation.",
      &max_nodes, ArgParser::INTEGER },
    { "--end","The end time for the simulation.",
      &end_time, ArgParser::INTEGER },
    { "", "", NULL, ArgParser::INVALID}
};


/* The main method for the Rollback simulation.

   The main emthod parses the command-line arguments, creates the
   agents, and runs the simulation.

   \param[in] argc The number of command-line arguments supplied to
   the simulation executable.

   \param[in] argv The actual command-line arguments supplied to the
   simulation executable.
*/
int main(int argc, char** argv) {
    const std::string HelpInfo =
	"A simulation that tends to be rollback heavy.\n"
	"Copyright (C) PC2Lab (http://pc2lab.ece.miamiOH.edu) 2012-\n"
	"Usage: rollbackSim [options]";
    ArgParser ap(arg_list, HelpInfo);
    ap.parseArguments(argc, argv, true);

    // first lets initialize the kernel
    muse::Simulation::initializeSimulation(argc, argv);
    // next get simulation kernel instance to work with
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    
    int agentsPerNode = max_agents / max_nodes;
    const int rank    = kernel->getSimulatorID(); 
    
    ASSERT(max_agents >= max_nodes);
    // check to make sure agents fit into nodes if uneven then last
    // node carries the extra agents.
    if ((rank == (max_nodes - 1)) && ((max_agents % max_nodes) > 0)) {
	// means we have to add the extra agents to the last kernel
	agentsPerNode = (max_agents / max_nodes) + (max_agents % max_nodes);
    }
    
    const int startID = (max_agents / max_nodes) * rank;
    muse::AgentID id  = -1u;
    for (int i = 0; (i < agentsPerNode); i++) {
        id = startID + i;
        RollbackAgent *rb_agent = new RollbackAgent(id, max_agents);
        kernel->registerAgent(rb_agent);
    } 
    
    // We set the start and end time of the simulation here
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);

    //we finally start the ping pong simulation here!!
    kernel->start();

    // now we finalize the kernel to make sure it cleans up.
    muse::Simulation::finalizeSimulation();

    // All done.
    return 0;
}

#endif
