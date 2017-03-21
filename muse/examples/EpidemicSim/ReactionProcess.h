/* 
 * File:   ReactionProcess.h
 * Author: Julius Higiro
 *
 * Created on March 20, 2017, 8:36 PM
 */

#ifndef REACTIONPROCESS_H
#define REACTIONPROCESS_H

#include <cmath>
#include <vector>
#include "Person.h"

class ReactionProcess {
    
public:
    
    /**
     * Models the evolution of the disease within individual entity in the 
     * population
     */
    ReactionProcess(){}
    
    /**
     * Defines the infection behavior of co-located individuals (probability of
     * an uninfected individual becoming infected). It determines the
     *  probability of transmission within the group of individuals at any
     * location. 
     */
    void reaction(){}
    
    /**
     * The probabilistic time transition system models the progression of 
     * the infection. A PTTS reflects the reaction that changes the state 
     * of an individual within the population.
     */
    void ptts(){}
    
private:
    
};


#endif /* REACTIONPROCESS_H */

