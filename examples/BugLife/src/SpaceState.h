/* 
  File:   SpaceState.h
  Author: Meseret R. Gebre     meseret.gebre@gmail.com
  
  This is the space state. We contain information about the space in this state.
*/

#ifndef SPACESTATE_H
#define	SPACESTATE_H

#include "State.h"
#include "DataTypes.h"
#include "BugDataTypes.h"

class SpaceState : public muse::State {
public:
    State* getClone();
    SpaceState();
    inline void    setFood(int f) { food=f; }
    inline int     getFood() const { return food; } 
    inline void    setBugID(muse::AgentID id) { bugID=id; }
    inline muse::AgentID getBugID() const { return bugID; } 
    inline void    setPredatorID(muse::AgentID id) { predatorID=id; }
    inline muse::AgentID getPredatorID() const { return predatorID; } 
protected:
    int food;
    muse::AgentID bugID;
    muse::AgentID predatorID;
};

#endif
