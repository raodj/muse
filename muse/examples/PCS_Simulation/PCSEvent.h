/* 
 * File:   PCSEvent.h
 * Author: Julius Higiro
 *
 * Created on August 31, 2016, 10:09 AM
 */

#ifndef PCSEVENT_H
#define PCSEVENT_H

#include "Event.h"

enum NextAction {
    NEXTCALL,
    MOVECALL,
    COMPLETECALL
};

enum Method {
    NEXT_CALL,
    MOVE_IN,
    MOVE_OUT,
    COMPLETE_CALL
};

/**
 * The PCSEvent class is used to create timestamped events that represent
 * a Portable in the PCS model. In similarity to a Portable, the PCSEvent object
 * has three independent exponentially distributed timestamp fields.
 **/
class PCSEvent : public muse::Event {
public:
    static PCSEvent* create(const muse::AgentID receiverID,
            const muse::Time recvTime, unsigned int moveTimeStamp,
            unsigned int callTimeStamp, unsigned int completionTimeStamp,
            Method method) {
        PCSEvent* event = reinterpret_cast<PCSEvent*> (new char[sizeof (PCSEvent)]);
        new (event) PCSEvent(receiverID, recvTime, moveTimeStamp, callTimeStamp,
                completionTimeStamp, method);
        return event;
    }
    inline unsigned int getMoveTimeStamp() const {return moveTimeStamp;}
    inline unsigned int getNextCallTimeStamp() const {return nextCallTimeStamp;}
    inline unsigned int getCompletionTimeStamp() const {return completionTimeStamp;}
    inline Method getMethod() const {return method;}
    
protected:
    unsigned int moveTimeStamp;
    unsigned int nextCallTimeStamp;
    unsigned int completionTimeStamp;
    Method method;
    PCSEvent(muse::AgentID receiver_id, muse::Time receive_time,
            unsigned int moveTimeStamp, unsigned int nextCallTimeStamp,
            unsigned int completionTimeStamp, Method method);
};


#endif /* PCSEVENT_H */

