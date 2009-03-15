/* 
  File:  Born.h
  Author: Meseret R. Gebre           meseret.gebre@gmail.com

  Lets a bug know that it was born. Little confusing, I know, but
  since this is an agent-based the only way to talk to agents is
  via events. Think of every event as a symbol in a language. This symbol
  can be a word or an entire book, depends on the information stored in the
  event and how the agents use it!
 */

#ifndef _BORN_H
#define	_BORN_H
 
#include "BugEvent.h"
using namespace muse; 

class Born : public BugEvent {
public:
    Born(AgentID receiverID,Time receiveTime, BugEventType e_type);
    inline int getEventSize() {return sizeof(Born); }
 
};

#endif	/*  _BORN_H */
