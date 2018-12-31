#ifndef PINGPONGAGENT_CPP
#define	PINGPONGAGENT_CPP

#include <iostream>
#include "PingPongAgent.h"
#include "Event.h"

PingPongAgent::PingPongAgent(const muse::AgentID& id) : 
    Agent(id, new muse::State()) {
    // Nothing else to be done in the constructor
}

void
PingPongAgent::initialize() {
    // This is a custom ping pong simulation, so we can assume only two agents!
    // the first agent is to start the ping
    if (getAgentID() == 0) {
        muse::Event* e = muse::Event::create(muse::AgentID(1), getTime() + 1);
        scheduleEvent(e);
    }  //end if
}  //end initialize

void
PingPongAgent::executeTask(const muse::EventContainer& events) {
    std::cout << "PingPong Agent [" << getAgentID() << "] @ LVT: "
              << getTime() << std::endl;
   
    if (!events.empty()){
        muse::Event * e = muse::Event::create((getAgentID() + 1) % 2,
                                              getTime() + 1);
        scheduleEvent(e);
    }
}  //end executeTask

void
PingPongAgent::finalize() {
    //cleanup and shutdown  
}  //end finalize

#endif //_PINGPONGAGENT_CPP

