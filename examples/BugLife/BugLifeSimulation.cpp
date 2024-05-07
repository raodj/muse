#ifndef BUG_LIFE_SIMULATION_CPP
#define BUG_LIFE_SIMULATION_CPP

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

#include "BugLifeSimulation.h"
#include "DataTypes.h"
#include "ArgParser.h"

#include "SpaceState.h"
#include "BugState.h"
#include "Space.h"
#include "Bug.h"
#include "oSimStream.h"
#include "MTRandom.h"

#include <cmath>
#include <cstdlib>
#include <vector>
#include <ostream>

using namespace muse;

BugLifeSimulation::BugLifeSimulation() {
    cols      = 3;   
    rows      = 3;   
    bugs      = 3;
    max_nodes = 1;
    end_time  = 10;
}

BugLifeSimulation::~BugLifeSimulation() {
    // Nothing much to be done here for this class.
}

void
BugLifeSimulation::processArgs(int& argc, char *argv[]) {
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
	{ "--cols", "The Number of columns in the space.", &cols,
	  ArgParser::INTEGER }, 
	{ "--rows", "The Number of rows in the space.",
	  &rows, ArgParser::INTEGER },
	{ "--bugs", "The number of bugs you want in the simulation.",
	  &bugs, ArgParser::INTEGER },
	{ "--end", "The end time for the simulation.", &end_time,
	  ArgParser::INTEGER },
	{"", "", NULL, ArgParser::INVALID}	 
    };

    // Let the kernel initialize using any additional command-line
    // arguments.
    muse::Simulation::initializeSimulation(argc, argv);

    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv ,true);
}

void
BugLifeSimulation::createMap() {
    AgentID id = 0;
    for (int x = 0; (x < cols); x++) {
        for (int y = 0; (y < rows); y++) {
            const coord location(x,y);
            coord_map[location] = id;
            id++;
        }
    }
}

void
BugLifeSimulation::createSpaces() {
    // Convenient local reference to simulation kernel
    Simulation* const kernel = Simulation::getSimulator();
    ASSERT( kernel != NULL );
    max_nodes = kernel->getNumberOfProcesses();
    ASSERT( max_nodes > 0 );
    const int max_space_agents = cols * rows;
    // Figure out number of agents per node.
    int space_agents_on_node = max_space_agents / max_nodes;
    // Any additional space agents are partitioned to the last compute node
    const int rank = kernel->getSimulatorID();
    if (rank == max_nodes - 1) {
	space_agents_on_node += max_space_agents % max_nodes;
    }
    // Create space agents and register them to kernels
    int space_id = (max_space_agents / max_nodes) * rank;
    for (int i = 0; (i < space_agents_on_node); i++, space_id++) {
        SpaceState *ss = new SpaceState();
        Space * space = new Space(space_id, ss);
        kernel->registerAgent(space);
    }
}

void
BugLifeSimulation::createBugs() {
    // Convenient local reference to simulation kernel
    Simulation* const kernel = Simulation::getSimulator();
    // Figure out number of bug-agents per node.
    int bug_agents_on_node = bugs / max_nodes;
    // Any additional space agents are partitioned to the last compute node
    const int rank = kernel->getSimulatorID();
    if (rank == max_nodes - 1) {
	bug_agents_on_node += bugs % max_nodes;
    }
    
    // Now we need to create and register the bugs
    const int max_space_agents = cols * rows; // ID's already used up
    int bug_id = max_space_agents + (bugs / max_nodes) * rank;
    for (int i = 0; (i < bug_agents_on_node); i++, bug_id++) {
        Bug* const bug = new Bug(bug_id, coord_map, cols, rows);
        kernel->registerAgent(bug);
    }
}

void
BugLifeSimulation::simulate() {
    // Convenient local reference to simulation kernel
    Simulation* const kernel = Simulation::getSimulator();    
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the kernel to make sure it cleans up.
    muse::Simulation::finalizeSimulation();
}

void
BugLifeSimulation::run(int &argc, char* argv[]) {
    ASSERT( argc > 0 );
    ASSERT( argv != NULL );
    // Create the singleton instance and parse command-line arguments.
    BugLifeSimulation blSim;
    blSim.processArgs(argc, argv);
    // Now create a coordinate map, space agents, and bug agents in
    // simulation using suitable helper methods.
    blSim.createMap();
    blSim.createSpaces();
    blSim.createBugs();
    // Now that the various agents for this parallel compute node have
    // been created, run the actual simulation.
    blSim.simulate();
}

// The main method that essentially transfers control to the
// BugLifeSimulation
int
main(int argc, char *argv[]) {
    BugLifeSimulation::run(argc, argv);
    return 0;
}

#endif
