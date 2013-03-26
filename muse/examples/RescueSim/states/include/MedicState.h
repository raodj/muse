
#ifndef MedicState_H
#define MedicState_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: MedicState.h
    Author: your name

    ........give brief description of what this  state contains here.......
*/

#include "State.h"
//#include "DataTypes.h"   //uncomment if you need muse data types

using namespace muse;
class MedicState : public State {

public:
    MedicState();
    State * getClone();
    ~MedicState();
};

#endif /* MedicState_H */
