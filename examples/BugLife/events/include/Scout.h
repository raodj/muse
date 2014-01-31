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
    static Scout* create(const muse::AgentID receiverID, const muse::Time receiveTime, BugEventType e_type) {
        Scout* event = reinterpret_cast<Scout*>(new char[sizeof(Scout)]);
        new (event) Scout(receiverID, receiveTime, e_type);
        return event;
    }
    inline int getEventSize() { return sizeof(Scout); }

    /** the space sets this value to report how much food is in the
	space at the time of receving this event.
	Version 11
     */
    int foodCount;
 protected:
    Scout(AgentID receiverID, Time receiveTime, BugEventType e_type);
};

#endif	/* _SCOUT_H */

