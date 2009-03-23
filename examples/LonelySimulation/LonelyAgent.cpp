#ifndef _LonelyAgent_CPP
#define	_LonelyAgent_CPP

#include "LonelyAgent.h"
#include "Event.h"


using namespace std;

LonelyAgent::LonelyAgent(AgentID& id, State* state) : Agent(id,state){}

void
LonelyAgent::initialize() throw (std::exception){
       
        Event * e = new Event(getAgentID(),getTime()+1); 
        if ( scheduleEvent(e) ) oss << "Talking to self: " << getAgentID() <<endl;
       
}//end initialize

void
LonelyAgent::executeTask(const EventContainer* events){
    if (!events->empty()){
        
        Event * e = new Event(getAgentID(),getTime()+1); 
        //if ( scheduleEvent(e) ) oss << "Talking to self: " << other_agent_id <<endl;
       
        //scheduleEvent(e);
        if ( scheduleEvent(e) ) oss << "Talking to self: " << getAgentID() <<endl;
        
    }
}//end executeTask

void
LonelyAgent::finalize() {
    //cout << "LonelyAgent [";cout <<getAgentID();cout << "] FINALIZE" << endl;
}//end finalize
#endif 

