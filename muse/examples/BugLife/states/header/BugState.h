/* 
  File:   BugState.h
  Author: Meseret R. Gebre     meseret.gebre@gmail.com
  
  This is the bug state. We contain information about the bug in this state.
  We care about the size of the bug and where in the space it is located.
 */

#ifndef _BUGSTATE_H
#define	_BUGSTATE_H

#include "State.h"
using namespace muse;

class BugState : public State {

public:
    State* getClone();
    BugState(int size, int x_pos, int y_pos);
    /** The size of the bug. 
	*/
    int size;

	/** This is the position of the bug in the space. 
	*/
	int x,y;
};

#endif	/* _BUGSTATE_H */

