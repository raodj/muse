R"(
#ifndef %CLASS_NAME%_CPP
#define %CLASS_NAME%_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "%CLASS_NAME%.h"
#include "%DISEASE_NAME%Agent.h"
#include "ArgParser.h"

%CLASS_NAME%::%CLASS_NAME%() {
    ode          = true;
    rows         = 1;
    cols         = 1;
    step         = 0.01;
    endTime      = 100;
}

%CLASS_NAME%::~%CLASS_NAME%() {
    // Nothing else to be done in the destructor.
}

void
%CLASS_NAME%::processArgs(int& argc, char* argv[]) {
    bool ssa = false;
    ArgParser::StringList initCompVals;
    ArgParser::ArgRecord arg_list[] = {
        {"--ssa", "Use SSA instead of ODE", &ssa, ArgParser::BOOLEAN},
        {"--step", "Time step for ODE/SSA operations",
         &step, ArgParser::DOUBLE},        
        {"--rows", "number of rows in simulation, dictates number of agents",
         &rows, ArgParser::INTEGER},
        {"--cols", "number of columns in simulation, dictates number of agents",
         &cols, ArgParser::INTEGER},
        {"--initVals", "initial compartment values in order", &initCompVals,
         ArgParser::STRING_LIST},
        {"--simEndTime", "Number of time steps in simulation",
         &endTime, ArgParser::INTEGER},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    const std::string copyright =
        "Heterogeneous Computing (HC) %DISEASE_NAME% Simulation\n"
        "Copyright (C) Miami University, Oxford, OHIO.";
    ArgParser ap(arg_list, copyright);
    ap.parseArguments(argc, argv, true);
    // Update the ode flag if ssa was chosen
    ode = !ssa;
    // Set size of simulation for agent's use
    %DISEASE_NAME%Agent::setParams(rows, cols);
    // Convert initial set of compartments to real values
    for (const std::string& str : initCompVals) {
        initVals.push_back(std::stof(str));
    }
}

void
%CLASS_NAME%::createAgents() {
    muse::Simulation* kernel = muse::Simulation::getSimulator();
    const int rank = kernel->getSimulatorID();  // MPI process rank
    // The math below evenly subdivides the agents between parallel
    // MPI processes and further between threads on each parallel
    // process.  Currently, it does not account for any skews in agent
    // distribution, and that needs to be updated.
    const int max_agents     = rows * cols;
    const int max_nodes      = kernel->getNumberOfProcesses();
    const int threadsPerNode = kernel->getNumberOfThreads();
    // const int skewAgents     = (max_nodes > 1) ? (max_agents * imbalance)
    //    : 0;
    // const double factor      = (skewAgents > 0) ?
    //    (skewAgents / (double) cumlSum(max_nodes)) : 0;
    const int skewAgents     = 0;
    const double factor      = 0;
    const int agentsPerNode  = (max_agents - skewAgents) / max_nodes;
    ASSERT(agentsPerNode > 0);
    const int agentsPerThread = std::max(1, agentsPerNode / threadsPerNode);
    ASSERT(agentsPerThread > 0);
    const int agentStartID   = (agentsPerNode * rank) + cumlSum(rank, factor);
    const int agentEndID     = (rank == max_nodes - 1) ? max_agents :
        ((agentsPerNode * (rank + 1)) + cumlSum(rank + 1, factor));

    // Now create the required number of agents on this parallel
    // process/thread.
    int currThread       = 0;  // current thread
    int currThrNumAgents = 0;  // number of agents on current thread.
    // Instantiate and register agents
    for (int i = agentStartID; (i < agentEndID); i++) {
        // Create a new agent
        const int x = i % cols, y = i / rows;
        %DISEASE_NAME%Agent* agent = new %DISEASE_NAME%Agent(i, x, y, ode, step,
                                                         initVals);
        kernel->registerAgent(agent, currThread);
        // Handle assigning agents to different threads
        if ((++currThrNumAgents >= agentsPerThread) &&
            (currThread < threadsPerNode - 1)) {
            currThread++;          // assign agents to next thread.
            currThrNumAgents = 0;  // reset number of agents on next thread.
        }
    }
    std::cout << "Rank " << rank << ": Registered agents from "
              << agentStartID    << " to "
              << agentEndID      << " agents.\n";
}

void
%CLASS_NAME%::simulate() {
    // Convenient local reference to simulation kernel
    muse::Simulation* const kernel = muse::Simulation::getSimulator();
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(endTime);
    // Finally start the simulation here!!
    kernel->start();
}

void
%CLASS_NAME%::run(int argc, char* argv[]) {
    // validate input
    ASSERT(argc > 0);
    ASSERT(argv != NULL);

    // Initialize MUSE and ensure it starts-up correctly.  On errors
    // the call below throws exceptions.
    muse::Simulation::initializeSimulation(argc, argv);

    // Create simulation, populate variables
    %CLASS_NAME% sim;
    sim.processArgs(argc, argv);
    // Create Agents
    sim.createAgents();
    // Run simulation
    sim.simulate();
    // Now we finalize the simulation to make sure it cleans up.
    muse::Simulation::finalizeSimulation();
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
    %CLASS_NAME%::run(argc, argv);
    return 0;
}

#endif
)"

