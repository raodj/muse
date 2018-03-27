#ifndef MUSE_OCLSTATE_CPP
#define MUSE_OCLSTATE_CPP
#include "oclState.h"
BEGIN_NAMESPACE(muse);

oclState::oclState(float population, float exp){
        //set initial seir values
	susceptible = population - exp;
	exposed = exp;
	infected = 0;
	recovered = 0;
}
END_NAMESPACE(muse);

#endif