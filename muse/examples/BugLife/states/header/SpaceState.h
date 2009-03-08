/* 
  File:   SpaceState.h
  Author: Meseret R. Gebre     meseret.gebre@gmail.com
  
  This is the space state. We contain information about the space in this state.
*/

#ifndef _SPACESTATE_H
#define	_SPACESTATE_H

#include "State.h"
#include "DataTypes.h"

using namespace muse;

class SpaceState : public State {

public:
    State* getClone();
    SpaceState(int food_count, int x_pos, int y_pos);
    int food,x,y;
    AgentID bugID,predatorID;
};

#endif	/* _SPACESTATE_H */

