#ifndef VictimState_H
#define VictimState_H

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
