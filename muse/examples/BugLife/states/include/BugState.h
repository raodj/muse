/* 
  File:   BugState.h
  Author: Meseret R. Gebre     meseret.gebre@gmail.com
  
  This is the bug state. We contain information about the bug in this state.
  We care about the size of the bug and where in the space it is located.
 */

#ifndef _BUGSTATE_H
#define _BUGSTATE_H

#include "State.h"
#include "BugDataTypes.h"

using namespace muse;

class BugState : public State {

public:
    State* getClone();
    BugState();
    inline coord getLocation() const {return location;}
    inline void setLocation(coord new_coord) {
      location.first  = new_coord.first;
      location.second = new_coord.second;
    }
    
    inline int getSize() const {return size;}
    inline void setSize(int new_size) {size=new_size;}
    
    inline bool isAlive() const {return alive;}
    inline void setIsAlive(bool life) {alive=life;}

    inline int getScoutReturned() const {return scoutReturned;}
    inline void setScoutReturned(int scouts) {scoutReturned = scouts;}

    inline std::pair<AgentID, int> getScoutSpace() const {return bestScoutSpace;}
    inline void setScoutSpace(std::pair<AgentID, int> ss) {
      bestScoutSpace.first = ss.first;
      bestScoutSpace.second = ss.second;
      
    }
    
 protected:
    /** The size of the bug. 
	*/
    int size;

    /** This is the position of the bug in the space. 
     */
    coord location;

    /** This is used to kill a bug and also used for birth
	Version 12
     */
    bool alive;

    /** This is used for the bug goal oriented movements
	once we get back all the scouts the bug make a choice of what space
	to move to.
	Version 11
     */
    int scoutReturned;
    std::pair<AgentID, int> bestScoutSpace;
};

#endif	/* _BUGSTATE_H */

