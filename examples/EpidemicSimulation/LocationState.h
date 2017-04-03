/* 
 * File:   LocationState.h
 * Author: Julius Higiro
 *
 * Created on April 1, 2017, 4:52 PM
 */

#ifndef LOCATIONSTATE_H
#define LOCATIONSTATE_H

#include "State.h"

class LocationState : public muse::State {

public:
    State * getClone();
    LocationState();
    
private:
        
    int index;
};

#endif /* LOCATIONSTATE_H */

