#ifndef _RoundRobinAgent_CPP
#define	_RoundRobinAgent_CPP

#include "RoundRobinAgent.h"
#include "Event.h"


using namespace std;

RoundRobinAgent::RoundRobinAgent(AgentID& id, State* state, int max_a) : Agent(id,state), max_agents(max_a) {}

void
RoundRobinAgent::initialize() throw (std::exception){
    if (getAgentID() == 0){
        AgentID other_agent_id = (getAgentID() + 1) % max_agents;
        Event * e = new Event(other_agent_id,getTime()+1); 
        scheduleEvent(e);
    }
}//end initialize

void
RoundRobinAgent::executeTask(const EventContainer* events){
    if (!events->empty()){
        AgentID other_agent_id = (getAgentID() + 1) % max_agents;
        Event * e = new Event(other_agent_id,getTime()+1); 
        // if ( scheduleEvent(e) ) oss << "Passed token to Round Robin Agent: " << other_agent_id <<endl;
        scheduleEvent(e);
    }
}//end executeTask

void
RoundRobinAgent::finalize() {
    
    //cout << "RoundRobinAgent [";cout <<getAgentID();cout << "] FINALIZE" << endl;
}//end finalize
#endif 

