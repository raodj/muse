
#ifndef VictimState_H
#define VictimState_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: VictimState.h
    Author: your name

    ........give brief description of what this  state contains here.......
*/

#include "State.h"
//#include "DataTypes.h"   //uncomment if you need muse data types

using namespace muse;
class VictimState : public State {

public:
    VictimState();
    State * getClone();
    ~VictimState();
};

#endif /* VictimState_H */
