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
// Authors: Harrison Roth          rothhl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------
#include "Simulation.h"
#include "OclExampleSimulation.h"
#include "OclState.h"
#include "ebola/EbolaAgent.h"
#include "synth/SynthAgent.h"
#include <algorithm>

/*
    The main method coordinates all the activtes of the
    simulation. Note that the main method runs on all the parallel
    processes used for simulation.

    \param[in] argc The number of command-line arguments.

    \param[in] argv The command-line arguments to be parsed
*/

OclExampleSimulation::OclExampleSimulation(int& argc, char* argv[]) {
    int row = 1;
    int col = 1;
    int pop = 250000;
    int exp = 30;
    int stop = 3;
    std::string cntry = "lib";
    bool useEbola = false;
    ArgParser::ArgRecord arg_list[] = {
        { "--row", "number of rows in simulation, dictates number of agents",
          &row, ArgParser::INTEGER},
        { "--col", "number of columns in simulation, dictates number of agents",
          &col, ArgParser::INTEGER},
        { "--pop", "Number of people initially in each agent",
          &pop, ArgParser::INTEGER},
        { "--exp", "Number of people exposed initially in each agent",
          &exp, ArgParser::INTEGER},
        { "--endtime", "Number of time steps in simulation",
          &stop, ArgParser::INTEGER},
        { "--country", "Country variables to use in ebola model",
          &cntry, ArgParser::STRING},
        { "--ebola", "Country variables to use in ebola model",
          &useEbola, ArgParser::BOOLEAN},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    rows = row;
    cols = col;
    popSize = pop;
    expSize = exp;
    stopTime = stop;
    ebola = useEbola;
    if(ebola){
        country = cntry;
        compartments = 6;
    }
    
}

void
OclExampleSimulation::createAgents() {
    muse::Simulation* kernel = muse::Simulation::getSimulator();
    const int max_nodes      = kernel->getNumberOfProcesses();
    const int threadsPerNode = kernel->getNumberOfThreads();
    const int skewAgents     = 0;
    const int agentsPerNode  = ((rows*cols) - skewAgents) / max_nodes;
    ASSERT(agentsPerNode > 0);
    const int agentsPerThread = std::max(1, agentsPerNode / threadsPerNode);
    ASSERT(agentsPerThread > 0);
    const int agentStartID   = 0;
    const int agentEndID     = rows * cols;
    // Create the agents and assign to multiple threads if that is the
    // configuration.
    int currThread       = 0;  // current thread
    int currThrNumAgents = 0;  // number of agents on current thread.
    for (int i = agentStartID; (i < agentEndID); i++) {
        muse::OclState* state = new muse::OclState(compartments, popSize, expSize);
        muse::OclAgent* agent;
        if (ebola){
            agent = new EbolaAgent(i, state, country);
        } else {
            agent = new SynthAgent(i, state);
        }    
        kernel->registerAgent(agent, currThread);
        agent->setKernel((muse::OclSimulation*)kernel);

        // Handle assigning agents to different threads
        if ((++currThrNumAgents >= agentsPerThread) &&
            (currThread < threadsPerNode - 1)) {
            currThread++;          // assign agents to next thread.
            currThrNumAgents = 0;  // reset number of agents on this thread.
        }
    }
}

int
main(int argc, char** argv) {
    OclExampleSimulation* sim = new OclExampleSimulation(argc, argv);
    muse::Simulation* kernel =
            muse::Simulation::initializeSimulation(argc, argv, true);
    if(!sim->ebola){
        muse::OclSimulation* ocl = (muse::OclSimulation*)kernel;
        sim->compartments = ocl->compartments;
    }
    sim->createAgents();
    kernel->setStartTime(0);
    kernel->setStopTime(sim->stopTime);
    kernel->start();
    return 0;
}
