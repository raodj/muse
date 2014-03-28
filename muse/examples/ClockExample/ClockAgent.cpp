#ifndef CLOCK_AGENT_CPP
#define	CLOCK_AGENT_CPP

#include "ClockAgent.h"
#include "Event.h"
#include <iostream>

ClockAgent::ClockAgent(muse::AgentID id) : Agent(id, new ClockState()) {
    // Nothing else to be done in the constructor
}

void
ClockAgent::initialize() throw (std::exception){
    oss << "Clock Agent initialized" << std::endl;
    // Here i will set the motion of scheduling event for the first
    // hour passage.  For this example we will assume that every FIVE
    // time steps an hour has stopped Therefore we will schedule the
    // first event for Time 5.
    Event * e = muse::Event::create(getAgentID(), getTime() + 5);
    scheduleEvent(e);
}

void
ClockAgent::executeTask(const muse::EventContainer* events){
    // when we receive a ClockEvent this means an hour has passed
    // print out our current LVT and print HOUR has passed message.
    // lastly, schedule an event to myself for the next hour.
    oss << "Hour passed @ LVT: " << getTime() << std::endl;
    // Here we are going through the EventContainer.  for the clock
    // example we should only have one event per "HOUR" == (every 5
    // time steps)
    ClockState * my_state = (ClockState*) getState();
    muse::EventContainer::const_iterator it;
    for (it = events->begin(); (it != events->end()); ++it) {
	muse::Event * e = muse::Event::create(getAgentID(), getTime() + 5);
        scheduleEvent(e);
        // We update our state here
        my_state->hour = (*it)->getReceiveTime();
    }//end for
}//end executeTask

void
ClockAgent::finalize() {
    //cleanup and shutdown
    oss << "Clock Agent finalized" << std::endl;
}//end finalize

#endif	

