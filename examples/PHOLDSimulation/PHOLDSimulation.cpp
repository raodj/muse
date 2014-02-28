#ifndef PHOLD_SIMULATION_CPP
#define PHOLD_SIMULATION_CPP

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

#include "Simulation.h"
#include "PHOLDSimulation.h"
#include "PHOLDAgent.h"
#include "PholdState.h"
#include "ArgParser.h"

PHOLDSimulation::PHOLDSimulation() {
    rows      = 3;
    cols      = 3;
    events    = 3;
    delay     = 1;
    max_nodes = 5;
    end_time  = 100;
}

PHOLDSimulation::~PHOLDSimulation() {}

void
PHOLDSimulation::processArgs(int argc, char** argv) {
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
        { "--cols", "The Number of columns in the space.", &cols,
            ArgParser::INTEGER },
        { "--rows", "The Number of rows in the space.",
            &rows, ArgParser::INTEGER },
        { "--delay", "The Delay time for the receive time, this will be max range for a random from [0,1]",
            &delay, ArgParser::INTEGER },
        { "--eventsPerAgent", "The number of bugs you want in the simulation.",
            &events, ArgParser::INTEGER },
        { "--computeNodes", "The max numbers of nodes used for this simulation.",
            &max_nodes, ArgParser::INTEGER },
        { "--simEndTime", "The end time for the simulation.", &end_time,
            ArgParser::INTEGER },
        {"", "", NULL, ArgParser::INVALID}
    };

    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv ,true);

    // Let the kernel initialize using any additional command-line
    // arguments.
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    kernel->initialize(argc, argv);

    max_agents = rows * cols;
    agentsPerNode = max_agents/max_nodes;
    rank = kernel->getSimulatorID();

    ASSERT(max_agents >= max_nodes);

    // ensure agents fit into nodes.
    //  - if uneven, then last node carries extra agents.
    if ( rank == (max_nodes-1) && (max_agents % max_nodes) > 0 ){
        agentsPerNode = (max_agents/max_nodes) + (max_agents % max_nodes);
    }
}

void
PHOLDSimulation::createAgents() {
    muse::AgentID id = -1u;
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    for (muse::AgentID i= 0;i < agentsPerNode; i++){
        PholdState * phold_state = new PholdState();
        id =  (max_agents/max_nodes)*rank + i;
        PHOLDAgent *phold_agent = new PHOLDAgent(id, phold_state, rows,
                                                 cols, events, delay);
        kernel->registerAgent(phold_agent);
        std::cout << kernel << std::endl;
    }
}

void
PHOLDSimulation::simulate() {
    // Convenient local reference to simulation kernel
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);
    kernel->setGVTDelayRate(4000);
    std::cout << "Calling kernel start." << std::endl;
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the kernel to make sure it cleans up.
    kernel->finalize();
}

void
PHOLDSimulation::run(int argc, char** argv) {
    // validate input
    ASSERT(argc > 0);
    ASSERT(argv != NULL);

    // Create simulation, populate variables
    PHOLDSimulation sim;
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
    PHOLDSimulation::run(argc, argv);
    return 0;
}

#endif
