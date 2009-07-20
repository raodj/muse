#ifndef _PHOLDSTATE_H
#define _PHOLDSTATE_H

#include "State.h"

using namespace muse;

class PholdState : public State {

public:
    State * getClone();
    PholdState();

    inline int getIndex() const {return index;}
    inline void setIndex(int new_index) {index=new_index;}
    
private:
    int index;
};

#endif	
