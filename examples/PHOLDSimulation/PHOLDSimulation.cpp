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
    rows           = 3;
    cols           = 3;
    events         = 3;
    delay          = 0;
    lookAhead      = 1;
    imbalance      = 0.0;
    selfEvents     = 0.0;
    end_time       = 100;
    granularity    = 0;
    delayDistrib   = "exponential";
    delayHist      = false;
    receiverRange  = 0;
    recvrDistrib   = "uniform";
    extraEventSize = 0;
    remoteEvents   = 0;
}

PHOLDSimulation::~PHOLDSimulation() {}

bool
PHOLDSimulation::processArgs(int argc, char** argv) {
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
        {"--delay-distrib", "The type of delay distribution to be used",
         &delayDistrib, ArgParser::STRING},
        {"--eventsPerAgent", "Initial number of events per agent",
         &events, ArgParser::INTEGER },
        {"--eventsPerLP", "Same as eventsPerAgent",
         &events, ArgParser::INTEGER },        
        {"--selfEvents", "Fraction of events agents send to themselves [0, 1]",
         &selfEvents, ArgParser::DOUBLE},
        {"--simEndTime", "The end time for the simulation.", &end_time,
         ArgParser::INTEGER },
        {"--imbalance", "Desired imbalance in partitioning [0, 0.99]",
         &imbalance, ArgParser::DOUBLE},
        {"--granularity", "Granularity (no units) per events", &granularity,
         ArgParser::LONG},
        {"--print-hist", "Print delay distribution histogram", &delayHist,
         ArgParser::BOOLEAN},
        {"--recvr-range", "Receiver agent ID range used with distribution",
         &receiverRange, ArgParser::INTEGER},
        {"--recvr-distrib", "Receiver agent ID distribution to be used",
         &recvrDistrib, ArgParser::STRING},
        {"--extra-evt-size", "Extra bytes to be added to each event",
         &extraEventSize, ArgParser::INTEGER},
        {"--remote-events", "%remote events when recvr-distrib is local_remote",
         &remoteEvents, ArgParser::DOUBLE},     
        {"", "", NULL, ArgParser::INVALID}
    };

    // Let the kernel initialize using any additional command-line
    // arguments.
    try {
        muse::Simulation::initializeSimulation(argc, argv);
    } catch (std::exception& exp) {
        std::cerr << "Exiting simulation due to initialization error: "
                  << exp.what() << std::endl;
        return false;
    }
    // Get the pointer to the simulation kernel we are to use.
    muse::Simulation* const kernel = muse::Simulation::getSimulator();

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
    // Ensure the delay distribution specified is indeed valid.
    PHOLDAgent::DelayType delayType = PHOLDAgent::toDelayType(delayDistrib);
    if (delayType == PHOLDAgent::INVALID_DELAY) {
        std::cerr << "Invalid delay distribution specified. Valid values are: "
                  << "uniform, poisson, exponential, reverse_poission, "
                  << "reverse_exponential.\n";
        return false;
    }
    // Ensure the delay distribution specified is indeed valid.
    delayType = PHOLDAgent::toDelayType(recvrDistrib);
    if (delayType == PHOLDAgent::INVALID_DELAY) {
        std::cerr << "Invalid recvr distribution specified. Valid values are: "
                  << "uniform, poisson, exponential, reverse_poission, "
                  << "reverse_exponential.\n";
        return false;
    }    
    // Everything went well.
    return true;
}

int
PHOLDSimulation::cumlSum(const int val, const double scale) const {
    return (scale * val * (val - 1) / 2);
}

void
PHOLDSimulation::createAgents() {
    muse::Simulation* kernel = muse::Simulation::getSimulator();    
    const int max_agents     = rows * cols;
    const int max_nodes      = kernel->getNumberOfProcesses();
    const int threadsPerNode = kernel->getNumberOfThreads();
    const int skewAgents     = (max_nodes > 1) ? (max_agents * imbalance) : 0;
    const double factor      = (skewAgents > 0) ?
        (skewAgents / (double) cumlSum(max_nodes)) : 0;
    const int agentsPerNode  = (max_agents - skewAgents) / max_nodes;
    ASSERT( agentsPerNode > 0 );    
    const int agentsPerThread= std::max(1, agentsPerNode / threadsPerNode);
    ASSERT( agentsPerThread > 0 );
    const int agentStartID   = (agentsPerNode * rank) + cumlSum(rank, factor);
    const int agentEndID     = (rank == max_nodes - 1) ? max_agents :
        ((agentsPerNode * (rank + 1)) + cumlSum(rank + 1, factor));
    // Converte distribution types from string to suitable enumeration.
    const PHOLDAgent::DelayType delayType =
        PHOLDAgent::toDelayType(delayDistrib);
    const PHOLDAgent::DelayType recvrType =
        PHOLDAgent::toDelayType(recvrDistrib);    
    // Create the agents and assign to multiple threads if that is the
    // configuration.
    int currThread       = 0;  // current thread
    int currThrNumAgents = 0;  // number of agents on current thread.
    int thrStartAgent    = agentStartID;  // First agent on current thread.
    for (int i = agentStartID; (i < agentEndID); i++) {
        PholdState* state = new PholdState();
        PHOLDAgent* agent = new PHOLDAgent(i, state, rows, cols, events, delay,
                                           lookAhead, selfEvents, granularity,
                                           delayType, receiverRange,
                                           recvrType, extraEventSize);
        // Setup range of local agents based on per-thread values.
        const int thrEndAgent = (currThread == threadsPerNode - 1) ?
            agentEndID : (thrStartAgent + agentsPerThread);
        agent->setLocalAgentRange(thrStartAgent, thrEndAgent, remoteEvents);
        kernel->registerAgent(agent, currThread);
        // Have the first agent print the delay histogram
        if (delayHist && (i == agentStartID)) {
            agent->printDelayDistrib(std::cout);
        }
        // Handle assigning agents to different threads
        if ((++currThrNumAgents >= agentsPerThread) &&
            (currThread < threadsPerNode - 1)) {
            currThread++;          // assign agents to next thread.
            currThrNumAgents = 0;  // reset number of agents on this thread.
            thrStartAgent = i + 1; // Set starting agent for new thread
        }
    }
    std::cout << "Rank " << rank << ": Registered agents from "
              << agentStartID    << " to "
              << agentEndID      << " agents.\n";
}

void
PHOLDSimulation::simulate() {
    // Convenient local reference to simulation kernel
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the simulation to make sure it cleans up.
    muse::Simulation::finalizeSimulation();
}

void
PHOLDSimulation::run(int argc, char** argv) {
    // validate input
    ASSERT(argc > 0);
    ASSERT(argv != NULL);

    // Create simulation, populate variables
    PHOLDSimulation sim;
    if (sim.processArgs(argc, argv)) {
        // Create Agents
        sim.createAgents();
        // Run simulation
        sim.simulate();
    }
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
