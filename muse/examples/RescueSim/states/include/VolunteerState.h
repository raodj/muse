
#ifndef VolunteerState_H
#define VolunteerState_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: VolunteerState.h
    Author: your name

    ........give brief description of what this  state contains here.......
*/

#include "State.h"
//#include "DataTypes.h"   //uncomment if you need muse data types

using namespace muse;
class VolunteerState : public State {

public:
    VolunteerState();
    State * getClone();
    ~VolunteerState();
};

#endif /* VolunteerState_H */
