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
#include "PholdState.h"
#include "DataTypes.h"
#include <math.h>
#include <cstdlib>
#include "ArgParser.h"

using namespace muse;
using namespace std;

class PHOLDSimulation {
    public:
        PHOLDSimulation();
        ~PHOLDSimulation();
        void processArgs(int argc, char** argv);
        void createAgents();
        void simulate();
        static void run(int argc, char** argv);
    private:
        int cols, rows, events, delay, max_nodes, end_time, max_agents, agentsPerNode, rank;

};

PHOLDSimulation::PHOLDSimulation() {
    rows = 3;
    cols = 3;
    events = 3;
    delay = 1;
    max_nodes = 5;
    end_time = 100;
}

PHOLDSimulation::~PHOLDSimulation() {}

void PHOLDSimulation::processArgs(int argc, char** argv) {
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
    Simulation* const kernel = Simulation::getSimulator();
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

void PHOLDSimulation::createAgents() {
    AgentID id = -1u;
    Simulation* const kernel = Simulation::getSimulator();
    
    for (AgentID i= 0;i < agentsPerNode; i++){
        PholdState * phold_state = new PholdState();
        id =  (max_agents/max_nodes)*rank + i;
        PHOLDAgent *phold_agent = new PHOLDAgent(id, phold_state,rows,cols,events,delay);
    
        cout << "Rank: " << rank << " is servicing lp: " << id << endl;
        kernel->registerAgent(phold_agent);
    }//end for

}

void PHOLDSimulation::simulate() {
    // Convenient local reference to simulation kernel
    Simulation* const kernel = Simulation::getSimulator();
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(end_time);
    kernel->setGVTDelayRate(4000);
    cout << "Calling kernel start." << endl;
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the kernel to make sure it cleans up.
    kernel->finalize();
}

void PHOLDSimulation::run(int argc, char** argv) {
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
*/
int main(int argc, char** argv) {
    PHOLDSimulation::run(argc, argv);
    return 0;
}

