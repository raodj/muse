#ifndef MUSE_OCLSTATE_H
#define MUSE_OCLSTATE_H
#include "State.h"
BEGIN_NAMESPACE(muse);

class oclState : public muse::State {
    friend class oclSimulation;
    friend class OCLAgent;
	float susceptible;
	float exposed;
	float infected;
	float recovered;
        oclState(float population, float exp);

};
END_NAMESPACE(muse);

#endif