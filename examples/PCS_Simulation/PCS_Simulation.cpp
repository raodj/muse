#ifndef PCS_SIMULATION_CPP
#define PCS_SIMULATION_CPP

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

/* 
 * File:   PCS_Simulation.cpp
 * Author: Julius Higiro
 *
 * Created on August 24, 2016, 12:28 PM
 */

#include "Simulation.h"
#include "PCS_Simulation.h"
#include "ArgParser.h"

using namespace std;

PCS_Simulation::PCS_Simulation() {
    rows             = 3;
    cols             = 3;
    events           = 3;
    delay            = 0;
    lookAhead        = 1;
    imbalance        = 0.0;
    selfEvents       = 0.0;
    end_time         = 100;
    granularity      = 0;
    channels         = 8;
    callIntervalMean = 5;
    callDurationMean = 3;
    moveIntervalMean = 9;   
}

PCS_Simulation::~PCS_Simulation() {}

bool
PCS_Simulation::processArgs(int argc, char** argv) {
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
        {"--cols", "The Number of columns in the space.", &cols,
         ArgParser::INTEGER },
        {"--rows", "The Number of rows in the space.",
         &rows, ArgParser::INTEGER },
        {"--lookahead", "Minimum delay time for events", &lookAhead,
         ArgParser::INTEGER},
        {"--delay", "The maximum random time added to look ahead [0, 100]",
         &delay, ArgParser::INTEGER },
        {"--eventsPerAgent", "Initial number of events per agent",
         &events, ArgParser::INTEGER },
        {"--selfEvents", "Fraction of events agents send to themselves [0, 1]",
         &selfEvents, ArgParser::DOUBLE},
        {"--simEndTime", "The end time for the simulation.", &end_time,
         ArgParser::INTEGER },
        {"--imbalance", "Desired imbalance in partitioning [0, 0.99]",
         &imbalance, ArgParser::DOUBLE},
        {"--granularity", "Granularity (no units) per events", &granularity,
         ArgParser::LONG},
        {"--maxChannels", "Maximum channels assigned to agents", &channels,
                 ArgParser::INTEGER},
        {"--callIntervalMean", "Average call interval time",
                &callIntervalMean, ArgParser::INTEGER},
        {"--callDurationMean", "Average call duration time",
                &callDurationMean, ArgParser::INTEGER},
        {"--moveIntervalMean", "Average move interval time",
                 &moveIntervalMean, ArgParser::INTEGER},
        {"", "", NULL, ArgParser::INVALID}
    };

    // Let the kernel initialize using any additional command-line
    // arguments.
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    try {
        kernel->initialize(argc, argv);
    } catch (std::exception& exp) {
        std::cerr << "Exiting simulation due to initialization error: "
                  << exp.what() << std::endl;
        return false;
    }

    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, true);
    
    rank = kernel->getSimulatorID();
    // Check to ensure we have at least as many agents as compute-nodes
    if (rows * cols < (int) kernel->getNumberOfProcesses()) {
        std::cerr << "The number of agents (i.e., rows * cols) must be "
                  << "greater than number of MPI processes.\n";
        return false;
    }
    // Everything went well.
    return true;
}

int
PCS_Simulation::cumlSum(const int val, const double scale) const {
    return (scale * val * (val - 1) / 2);
}

void
PCS_Simulation::createAgents() {
    muse::Simulation* kernel = muse::Simulation::getSimulator();    
    const int max_agents     = rows * cols;
    const int max_nodes      = kernel->getNumberOfProcesses();
    const int skewAgents     = (max_nodes > 1) ? (max_agents * imbalance) : 0;
    const double factor      = (skewAgents > 0) ?
        (skewAgents / (double) cumlSum(max_nodes)) : 0;
    const int agentsPerNode  = (max_agents - skewAgents) / max_nodes;
    ASSERT( agentsPerNode > 0 );
    const int agentStartID   = (agentsPerNode * rank) + cumlSum(rank, factor);
    const int agentEndID     = (rank == max_nodes - 1) ? max_agents :
        ((agentsPerNode * (rank + 1)) + cumlSum(rank + 1, factor));
    
    for (int i = agentStartID; (i < agentEndID); i++) {
        PCS_State* state = new PCS_State();
        PCSAgent* agent = new PCSAgent(i, state, rows, cols, events, delay,
                channels, callIntervalMean, callDurationMean, moveIntervalMean,
                lookAhead, selfEvents, granularity);
        kernel->registerAgent(agent);
    }
    std::cout << "Rank " << rank << ": Registered agents from "
              << agentStartID    << " to "
              << agentEndID      << " agents.\n";
}

void
PCS_Simulation::simulate() {
    // Convenient local reference to simulation kernel
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the kernel to make sure it cleans up.
    kernel->finalize();
}

void
PCS_Simulation::run(int argc, char** argv) {
    // validate input
    ASSERT(argc > 0);
    ASSERT(argv != NULL);
    // Create simulation, populate variables
    PCS_Simulation sim;
    if (sim.processArgs(argc, argv)) {
        // Create Agents
        sim.createAgents();
        // Run simulation
        sim.simulate();
    }
}

int 
main(int argc, char** argv) {
    PCS_Simulation::run(argc, argv);
    return 0;
}

#endif
