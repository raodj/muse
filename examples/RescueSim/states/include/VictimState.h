
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
using namespace muse;

class VictimState : public State {
public:
   VictimState();
   State * getClone();
   inline void setFound() {found = true;}
protected:
   bool found;
};

#endif /* VictimState_H */
