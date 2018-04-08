#ifndef MUSE_OCLSTATE_CPP
#define MUSE_OCLSTATE_CPP
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

END_NAMESPACE(muse);

#endif