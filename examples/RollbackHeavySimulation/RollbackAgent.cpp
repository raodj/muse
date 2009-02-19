#ifndef _PINGPONGAGENT_CPP
#define	_PINGPONGAGENT_CPP

#include "RollbackAgent.h"
#include "Event.h"
#include <iostream>

using namespace std;

RollbackAgent::RollbackAgent(AgentID& id, State* state, int max_a) : Agent(id,state), max_agents(max_a) {}

void
RollbackAgent::initialize() throw (std::exception){
    if (getAgentID() == 0){
        Time sent=0, receive=1;
        for (int i = 0; i < max_agents; i++){

            //if (getAgentID() != AgentID(i)){
                Event * e = new Event(getAgentID(),AgentID(i),sent, receive); 
                scheduleEvent(e);
                //}
        }
    }
}//end initialize

void
RollbackAgent::executeTask(const EventContainer* events){
    if (!events->empty()){
        Time sent=(*events->begin())->getReceiveTime(),
             receive=(*events->begin())->getReceiveTime()+1;

       for (int i = 0; i < max_agents; i++){
          
           //if (getAgentID() != AgentID(i)){
               Event * e = new Event(getAgentID(),AgentID(i),sent, receive);
          
           
               // if (scheduleEvent(e)) cout << "RA: " << *e << endl;
               scheduleEvent(e);
               // }
       }
    }
}//end executeTask

void
RollbackAgent::finalize() {
    
    cout << "RollbackAgent [";cout <<getAgentID();cout << "] FINALIZE" << endl;
}//end finalize
#endif 

