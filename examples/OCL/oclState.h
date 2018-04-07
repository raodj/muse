#ifndef MUSE_OCLSTATE_H
#define MUSE_OCLSTATE_H
#include "State.h"
BEGIN_NAMESPACE(muse);

class oclState : public muse::State {
    friend class oclSimulation;
    friend class OCLAgent;
        int compartments;
        float* values;
        oclState(int compartmentNum, float population, float exp);
        ~oclState();
};
END_NAMESPACE(muse);

#endif