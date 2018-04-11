#ifndef MUSE_OCLSTATE_CPP
#define MUSE_OCLSTATE_CPP

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
#include "oclState.h"
BEGIN_NAMESPACE(muse);

oclState::oclState(int compartmentNum, float population, float exp): compartments(compartmentNum), values(new float[compartments]){
        // set initial seir values
    for(int i = 0; i < compartments; i++){
        values[i] = 0;
    }
    values[0] = population;
    values[1] = exp;
}

 
oclState* oclState::clone(){
    // Go through and copy all compartment values to the passed in state
    oclState* clone = new oclState(compartments, 0,0);
    for(int i = 0; i < compartments; i++){
        clone->values[i] = values[i];
    }
    return clone;
}

END_NAMESPACE(muse);
#endif