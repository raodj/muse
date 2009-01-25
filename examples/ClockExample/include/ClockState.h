/* 
 * File:   ClockState.h
 * Author: gebremr
 *
 * Created on December 10, 2008, 11:29 PM
 */

#ifndef _CLOCKSTATE_H
#define	_CLOCKSTATE_H

#include "State.h"
using namespace muse;

class ClockState : public State {

public:
    State* getClone();
    ClockState();
    //this is the only information that is changing in the clock simulation.
    Time hour;
};

#endif	/* _CLOCKSTATE_H */

