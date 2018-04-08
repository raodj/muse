#ifndef MUSE_OCLSTATE_H
#define MUSE_OCLSTATE_H
#include "State.h"
#include <iostream>
#include <memory>
BEGIN_NAMESPACE(muse);

class oclState : public muse::State {
    friend class oclSimulation;
    friend class OCLAgent;
        int compartments;
        std::unique_ptr<float[]> values;
        oclState(int compartmentNum, float population, float exp);
};
END_NAMESPACE(muse);

#endif