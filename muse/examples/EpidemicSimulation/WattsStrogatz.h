/* 
 * File:   WattsStrogatz.h
 * Author: Julius Higiro
 *
 * Created on March 20, 2017, 8:39 PM
 */

#ifndef WATTSSTROGATZ_H
#define WATTSSTROGATZ_H

#define PRECISION 10000

#include "Agent.h"

class WattsStrogatz {
    
public:
    
/**
     * A graph that represents a small world social network model.
     * \param[in] K - number of neighbors, Beta - probability of rewiring edge,
     * Rows - total # of rows in sim. model and Cols - total # of cols. in sim.
     * model.
     */
    WattsStrogatz(muse::AgentID id, int K, float Beta, const size_t Rows,
            const size_t Cols) :
    k(K), beta(Beta), rows(Rows), cols(Cols), generator(id) {}
    
    /**
     * Build a lattice ring comprised of N (row x col) nodes. 
     * Every node is connected to it's k/2 neighbors on each side.
     */
    void generateModel() {
        
        int left = k/2;
        int right = k - left;
        Matrix world(rows, std::vector<bool>(cols));
        for (size_t row = 0; row < (rows*cols); row++) {
            for (size_t col = 0; col < (rows*cols); col++) {
                world[row][col] = false;
            }
        }
        
        for (size_t i = 0; i < (rows*cols); i++) {
            // Connect each node with neighbors on the left.
            for (size_t j = 1; j <= left; j++) {
                int leftIndex = (i + (rows*cols) - j) % (rows*cols);
                world[i][leftIndex] = true;
                world[leftIndex][i] = true;
            }
            
            // Connect each node with neighbors on the right.
            for (size_t k = 1; k <= right; k++) {
                int rightIndex = (i + k) % (rows*cols);
                world[i][rightIndex] = true;
                world[rightIndex][i] = true;
            }
        }
        createRandomNetworkLinks(world);
    }
    
    /**
     * Randomly rewire each node to generate a social network model such that
     * replicates and self connections are excluded.
     */
    void createRandomNetworkLinks(Matrix world) {
        std::uniform_distribution<unsigned int> beta_distrib(0, PRECISION-1);
        std::uniform_distribution<unsigned int> node_distrib(0, (row*col)-1);
        for (size_t row = 0; row < (rows * cols); row++) {
            for (size_t col = 0; col < (rows * cols); col++) {
                if (!world[row][col]) {
                    continue;
                }
                float randNum = beta_distrib(generator);
                if (randNum >= beta) {
                    continue;
                }
                size_t index = 0;
                while (true) {
                    index = node_distrib(generator);
                    if ((index != row) && (index != col)) {
                        break;
                    }
                }
                world[row][col] = false;
                world[col][row] = false;
                world[row][index] = false;
                world[index][row] = false;
            }
        }
    }
    
    
private:
    
    /**
     * The number of neighbors that each node assumes a connection.
     */
    int k;
    
    /**
     * The total number of rows in the simulation model.
     */
    const size_t rows;
    
    /**
     * The total number of columns in the simulation model.
     */
    const size_t cols;
        
    /**
     * The probability value that is used to determine rewiring.
     */
    float beta;
    
    /** 
     * The engine that generates pseudo-random numbers.
     */
    std::mt19937 generator;
    
    /**
     * The world that represents a social network.
     */
    using Matrix = std::vector<std::vector<bool>>;
};


#endif /* WATTSSTROGATZ_H */

