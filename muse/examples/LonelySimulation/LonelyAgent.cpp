#ifndef _LonelyAgent_CPP
#define	_LonelyAgent_CPP

#include "LonelyAgent.h"
#include "Event.h"


using namespace std;

LonelyAgent::LonelyAgent(AgentID& id, State* state) : Agent(id,state){}

void
LonelyAgent::initialize() throw (std::exception){
        Time  receive=1;
        Event * e = new Event(getAgentID(),receive); 
        //if ( scheduleEvent(e) ) oss << "Talking to self: " << other_agent_id <<endl;
        //else cout << "Scheduling event : FAILED" <<endl;
        scheduleEvent(e);
}//end initialize

void
LonelyAgent::executeTask(const EventContainer* events){
    if (!events->empty()){
        Time receive= (*events->begin())->getReceiveTime()+1;
        Event * e = new Event(getAgentID(),receive); 
        //if ( scheduleEvent(e) ) oss << "Talking to self: " << other_agent_id <<endl;
        //else cout << "Scheduling event : FAILED" <<endl;
        scheduleEvent(e);
    }
}//end executeTask

void
LonelyAgent::finalize() {
    //cout << "LonelyAgent [";cout <<getAgentID();cout << "] FINALIZE" << endl;
}//end finalize
#endif 

