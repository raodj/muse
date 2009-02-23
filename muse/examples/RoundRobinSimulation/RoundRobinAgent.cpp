#ifndef _RoundRobinAgent_CPP
#define	_RoundRobinAgent_CPP

#include "RoundRobinAgent.h"
#include "Event.h"
#include <iostream>

using namespace std;

RoundRobinAgent::RoundRobinAgent(AgentID& id, State* state, int max_a) : Agent(id,state), max_agents(max_a) {}

void
RoundRobinAgent::initialize() throw (std::exception){
    if (getAgentID() == 0){
        Time sent=0, receive=1;
        AgentID other_agent_id = (getAgentID() + 1) % max_agents;
        //cout << "OTHER AGENT ID: " << other_agent_id <<endl;
        Event * e = new Event(getAgentID(),other_agent_id,sent, receive); 
        if ( scheduleEvent(e) ) cout << "Passed token to Round Robin Agent: " << other_agent_id <<endl;
        else cout << "Scheduling event : FAILED" <<endl;
    }
}//end initialize

void
RoundRobinAgent::executeTask(const EventContainer* events){
    if (!events->empty()){
        Time sent=(*events->begin())->getReceiveTime(),
             receive=(*events->begin())->getReceiveTime()+1;

        cout << "GOT TOKEN @ TIME: " << sent <<endl;
        
        AgentID other_agent_id = (getAgentID() + 1) % max_agents;
        //cout << "OTHER AGENT ID: " << other_agent_id <<endl;
        Event * e = new Event(getAgentID(),other_agent_id,sent, receive); 
        if ( scheduleEvent(e) ) cout << "Passed token to Round Robin Agent: " << other_agent_id <<endl;
        else cout << "Scheduling event : FAILED" <<endl;
       
    }
}//end executeTask

void
RoundRobinAgent::finalize() {
    
    //cout << "RoundRobinAgent [";cout <<getAgentID();cout << "] FINALIZE" << endl;
}//end finalize
#endif 

