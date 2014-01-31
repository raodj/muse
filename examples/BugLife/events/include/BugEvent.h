#ifndef _BUGEVENT_H
#define _BUGEVENT_H
 
#include "Event.h"
#include "BugDataTypes.h"
using namespace muse; 

class BugEvent : public Event {
public:
    static BugEvent* create(const muse::AgentID receiverID, const muse::Time receiveTime, BugEventType e_type) {
        BugEvent* event = reinterpret_cast<BugEvent*>(new char[sizeof(BugEvent)]);
        new (event) BugEvent(receiverID, receiveTime, e_type);
        return event;
    }
    inline int getEventSize() { return sizeof(BugEvent); }
    inline BugEventType getEventType()const { return event_type;}
 protected:
    BugEventType event_type;
    BugEvent(AgentID receiverID, Time receiveTime,BugEventType e_type);
};

#endif	/* _BUGEVENT_H */

