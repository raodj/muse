/* 
  File:   SpaceState.h
  Author: Meseret R. Gebre     meseret.gebre@gmail.com
  
  This is the space state. We contain information about the space in this state.
*/

#ifndef _SPACESTATE_H
#define	_SPACESTATE_H

#include "State.h"
#include "DataTypes.h"
#include "BugDataTypes.h"

using namespace muse;

class SpaceState : public State {

public:
    State* getClone();
    SpaceState();
    inline void setFood(int f) {food=f;}
    inline int  getFood() const {return food;} 
    inline void setBugID(AgentID id) {bugID=id;}
    inline int  getBugID() const {return bugID;} 
    inline void setPredatorID(AgentID id) {predatorID=id;}
    inline int  getPredatorID() const {return predatorID;} 
protected:
    int food;
    AgentID bugID,predatorID;
};

#endif	/* _SPACESTATE_H */

