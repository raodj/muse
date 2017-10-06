#ifndef ROUND_ROBIN_SIMULATION_CPP
#define ROUND_ROBIN_SIMULATION_CPP

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "RoundRobinAgent.h"
#include "RoundRobinSimulation.h"
#include "Simulation.h"
#include "DataTypes.h"
#include "ArgParser.h"
#include <vector>
#include <stdlib.h>
#include <iostream>

const std::string RoundRobinSimulation::HelpInfo =
    "A simulation in which a token is exchanged in a round-robin manner.\n"
    "Copyright (C) PC2Lab (http://pc2lab.ece.miamiOH.edu) 2012-\n"
    "Usage: roundRobinSim [options]";

RoundRobinSimulation::RoundRobinSimulation() {
    max_agents = 3;
    max_nodes  = 1;
    end_time   = 10;
}

RoundRobinSimulation::~RoundRobinSimulation() {
    // Nothing else to be done here.
}

void
RoundRobinSimulation::processArgs(int argc, char** argv) {
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
        { "--agents", "Number of agents in simulation", &max_agents,
	  ArgParser::INTEGER },
        { "--endTime", "The end time for the simulation.", &end_time,
	  ArgParser::INTEGER },
        {"", "", NULL, ArgParser::INVALID}
    };

    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list, HelpInfo);
    ap.parseArguments(argc, argv ,true);

    // Let the kernel initialize using any additional command-line
    // arguments.
    muse::Simulation::initializeSimulation(argc, argv);
}

void
RoundRobinSimulation::createAgents() {
    // First compute the number of agents to be created on this node.
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    max_nodes = kernel->getNumberOfProcesses();
    ASSERT( max_nodes > 0 );
    const int rank    = kernel->getSimulatorID();
    int agentsPerNode = max_agents / max_nodes;
    if (rank == (max_nodes - 1)) {
	agentsPerNode += max_agents % max_nodes;
    }
    // Compute the starting ID for agents on this node.
    const muse::AgentID startID = (max_agents / max_nodes) * rank;
    for (int i = 0; (i < agentsPerNode); i++) {
	const muse::AgentID id = startID + i;
        RoundRobinAgent *agent = new RoundRobinAgent(id, max_agents);
        kernel->registerAgent(agent);
    }
}

void
RoundRobinSimulation::simulate() {
    // Convenient local reference to simulation kernel
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);
    kernel->setGVTDelayRate(4000);
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the kernel to make sure it cleans up.
    muse::Simulation::finalizeSimulation();
}

void
RoundRobinSimulation::run(int argc, char** argv) {
    // validate input
    ASSERT(argc > 0);
    ASSERT(argv != NULL);

    // Create simulation, populate variables
    RoundRobinSimulation sim;
    sim.processArgs(argc, argv);

    // Create Agents
    sim.createAgents();
    // Run simulation
    sim.simulate();
}

/*
    The main method coordinates all the activtes of the
    simulation. Note that the main method runs on all the parallel
    processes used for simulation.

    \param[in] argc The number of command-line arguments.

    \param[in] argv The command-line arguments to be parsed
*/
int
main(int argc, char** argv) {
    RoundRobinSimulation::run(argc, argv);
    return 0;
}

#endif
