#ifndef MUSE_OCLSTATE_H
#define MUSE_OCLSTATE_H

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

#include "State.h"
#include <iostream>
#include <memory>
BEGIN_NAMESPACE(muse);

// Use real for all state data so users can redefine real as anything needed
typedef float real;

class OclState : public muse::State {
    // friend classes
//    friend class oclSimulation;
    friend class OclAgent;
public:
    // class variables
    int compartments;
    std::vector<real> values;

    /*
     * Constructor - Create oclState
     * 
     * \param compartmentNum: the number of compartments in the
     * state, should relate to the number of compartments in the
     * model being used.
     * 
     * \param population: total population of this agent
     * 
     * \param exp: the number of initially exposed in this agent
     */
    OclState(int compartmentNum, real population, real exp);

    /*
     * Copy this state into another state
     * 
     * \param cp: the state that data is copied into from this state
     */
    oclState* clone();
};
END_NAMESPACE(muse);

#endif