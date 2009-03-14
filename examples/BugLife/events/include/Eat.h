/* 
  File:   Eat.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  This event is used to request a eat from a space. A bug must send this event
  and the space will either accept/decline based on some constraint. 
 */

#ifndef _EAT_H
#define	_EAT_H
 
#include "BugEvent.h"
using namespace muse; 

class Eat : public BugEvent {
public:
    Eat(AgentID receiverID,Time receiveTime, BugEventType e_type);
    inline int getEventSize() {return sizeof(Eat); }
    //the space agent use this to set how much a bug can eat from the space.
    //this is used by the bug to figure out how much to grow
    inline void setEatAmount(int amount) {eatAmount=amount;}
    int eatAmount;
};

#endif	/* _EAT_H */

