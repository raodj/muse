#ifndef _CLOCKEVENT_CPP
#define	_CLOCKEVENT_CPP

#include "ClockAgent.h"
#include "Event.h"
#include <iostream>

using namespace std;

ClockAgent::ClockAgent(AgentID id, ClockState* state) : Agent(id,state) {}

void
ClockAgent::initialize() throw (std::exception){
    cout << "Clock Agent INITIALIZE" << endl;
    //here i will set the motion of scheduling event for the first hour passage.
    //For this example we will assume that every FIVE time steps an hour has stopped
    //Therefore we will schedule the first event for Time 5.
    Event * e = new Event(getAgentID(), Time(getTime()+5));
    scheduleEvent(e);
}//end initialize

void
ClockAgent::executeTask(const EventContainer* events){
    //when we receive a ClockEvent this means an hour has passed
    //print out our current LVT
    //and print HOUR has passed message.
    //lastly, schedule an event to myself for the next hour.
    cout << "HOUR PASSED @ LVT: " << getTime() << endl;
    
    //here we are going through the EventContainer.
    //for the clock example we should only have one event per "HOUR" == (every 5 time steps)
    ClockState * my_state = (ClockState*)getState();
    EventContainer::const_iterator it;
    for (it=events->begin(); it != events->end(); ++it ){
        Event * e = new Event(getAgentID(), getTime()+5);
        scheduleEvent(e);
        //we update our state here
        my_state->hour = (*it)->getReceiveTime();
    }//end for

}//end executeTask

void
ClockAgent::finalize() {
    //cleanup and shutdown
    cout << "Clock Agent FINALIZE" << endl;

}//end finalize
#endif	/* _CLOCKEVENT_H */

