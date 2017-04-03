/* 
 * File:   EpidemicEvent.h
 * Author: Julius Higiro
 *
 * Created on March 26, 2017, 4:38 PM
 */

#ifndef EPIDEMICEVENT_H
#define EPIDEMICEVENT_H

#include "Event.h"
#include "Person.h"

enum EventType {
    ARRIVAL,
    DEPARTURE 
};

class EpidemicEvent : public muse::Event {
public:

    static EpidemicEvent* create(const muse::AgentID receiverID,
            const muse::Time recvTime, unsigned int locArrivalTimeStamp,
            Person person, EventType event_type) {
        EpidemicEvent* event =
                reinterpret_cast<EpidemicEvent*> (new char[sizeof (EpidemicEvent)]);
        new (event) EpidemicEvent(receiverID, recvTime, locArrivalTimeStamp,
                person, event_type);
        return event;
    }
    
    inline unsigned int getMoveTimeStamp() const { return locArrivalTimeStamp; }
    inline EventType getEventType() const { return event_type; }
    
protected:
    unsigned int locArrivalTimeStamp;
    Person person;
    EventType event_type;
    EpidemicEvent(muse::AgentID receiver_id, muse::Time receive_time,
            unsigned int locArrivalTimeStamp, Person person, EventType event_type);

};

#endif /* EPIDEMICEVENT_H */

