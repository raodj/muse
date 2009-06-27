#ifndef _PINGPONGAGENT_CPP
#define	_PINGPONGAGENT_CPP

#include "PingPongAgent.h"
#include "Event.h"
#include <iostream>

using namespace std;

PingPongAgent::PingPongAgent(AgentID& id, State* state) : Agent(id,state) {}

void
PingPongAgent::initialize() throw (std::exception){
    
   //this is a custom pingpong simulation, so we can assume only two agents!
    //the first agent is to start the ping
    if (getAgentID() == 0){
       
        Event * e = Event::create(AgentID(1),getTime()+1);
        this->scheduleEvent(e);
        //cout << "Sent ---> PING" << endl;
    }//end if
}//end initialize

void
PingPongAgent::executeTask(const EventContainer* events){
    cout << "PingPong Agent [";cout <<getAgentID();cout << "] @ LVT: ";cout << this->getTime() << endl;
   
    if (!events->empty()){
        
        Event * e = Event::create( (getAgentID()+1)%2 ,getTime()+1);
        this->scheduleEvent(e);
        /* if (getAgentID() == 1){
            cout << "Got  ---> PING" << endl;
            cout << "Sent ---> PONG" << endl;
        }else{
            cout << "Got  ---> PONG" << endl;
            cout << "Sent ---> PING" << endl;
            }*/
    }
}//end executeTask

void
PingPongAgent::finalize() {
    //cleanup and shutdown
  
}//end finalize
#endif //_PINGPONGAGENT_CPP

