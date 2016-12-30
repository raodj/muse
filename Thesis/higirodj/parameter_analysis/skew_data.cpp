#include <iostream>

// This is a simple C++ program that prints the partitioning of agents
// in PHOLD as the Skew value changes.  This is used to generate data
// for plotting skew charts.

// NOTE: The partition code/logic in createAGents() is (and should be)
// copy-paste from PHOLDSimulation::createAgents() so that the results
// here are consistent with the operation of PHOLD benchmark

// Convenience method to return cumulative sum of upto (n-1), that is
// given n, this method returns: [1 + 2 + .. + (n - 1)] * scale
int cumlSum(const int val, const double scale = 1) {
    return (scale * val * (val - 1) / 2);
}

void createAgents(const int max_agents, const int max_nodes,
                  const double imbalance) {
    // This code is (and should be) copy-paste from
    // PHOLDSimulation::createAgents() so that the results here are
    // consistent with the operation of PHOLD benchmark
    const int skewAgents     = (max_nodes > 1) ? (max_agents * imbalance) : 0;
    const double factor      = (skewAgents > 0) ?
        (skewAgents / (double) cumlSum(max_nodes)) : 0;
    const int agentsPerNode  = (max_agents - skewAgents) / max_nodes;
    std::cout << imbalance;
    for (int rank = 0; (rank < max_nodes); rank++) {
        const int agentStartID   = (agentsPerNode * rank) +
            cumlSum(rank, factor);
        const int agentEndID     = (rank == max_nodes - 1) ? max_agents :
            ((agentsPerNode * (rank + 1)) + cumlSum(rank + 1, factor));
        const int numAgents = agentEndID - agentStartID;
        std::cout << ", " << numAgents;
    }
    std::cout << std::endl;    
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: <MaxAgents> <MaxMPIprocs>\n";
        return 1;
    }
    // Process command-line arguments
    const int max_agents   = std::stoi(argv[1]);
    const int max_nodes    = std::stoi(argv[2]);

    // Print data with imbalance changing by 10%
    for (double imbalance = 0; (imbalance <= 1.0); imbalance += 0.1) {
        createAgents(max_agents, max_nodes, imbalance);
    }
    
    return 0;
}
