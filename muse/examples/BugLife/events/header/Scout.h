/* 
  File:   Scout.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This event is used for a bug to scout the spaces around. Things 
  of interest to the bug would include the food count and predator
  is in the space.
 */

#ifndef _SCOUT_H
#define	_SCOUT_H
 
#include "BugEvent.h"
using namespace muse; 

class Scout : public BugEvent {
public:
    Scout(AgentID receiverID,Time receiveTime, int e_type);
    inline int getEventSize() {return sizeof(Scout); }
};

#endif	/* _SCOUT_H */

