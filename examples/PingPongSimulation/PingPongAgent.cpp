#ifndef _PINGPONGAGENT_CPP
#define	_PINGPONGAGENT_CPP

#include "PingPongAgent.h"
#include "Event.h"
#include <iostream>

using namespace std;

PingPongAgent::PingPongAgent(AgentID& id, State* state) : Agent(id,state) {}

void
PingPongAgent::initialize() throw (std::exception){
    cout << "PingPong Agent ["; cout <<getAgentID() ; cout << "] INITIALIZE" << endl;
   //this is a custom pingpong simulation, so we can assume only two agents!
    //the first agent is to start the ping
    if (getAgentID() == 1){
        Time sent=0, receive=1;
        AgentID other_agent = 2;
        Event * e = new Event(getAgentID(),other_agent,sent, receive);
        this->scheduleEvent(e);
        cout << "Sent ---> PING" << endl;
    }//end if
}//end initialize

void
PingPongAgent::executeTask(const EventContainer* events){
    cout << "PingPong Agent [";cout <<getAgentID();cout << "] @ LVT: ";cout << this->getLVT() << endl;
    AgentID other_agent = 1;
    if (getAgentID() == 1){
      other_agent = 2;
    }
    if (!events->empty()){
        Time sent=(*events->begin())->getReceiveTime(),
             receive=(*events->begin())->getReceiveTime()+1;
        Event * e = new Event(getAgentID(),other_agent,sent, receive);
        this->scheduleEvent(e);
        if (getAgentID() == 2){
            cout << "Got  ---> PING" << endl;
            cout << "Sent ---> PONG" << endl;
        }else{
            cout << "Got  ---> PONG" << endl;
            cout << "Sent ---> PING" << endl;
        }
    }
}//end executeTask

void
PingPongAgent::finalize() {
    //cleanup and shutdown
    cout << "PingPong Agent [";cout <<getAgentID();cout << "] FINALIZE" << endl;
}//end finalize
#endif //_PINGPONGAGENT_CPP

