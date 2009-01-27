#ifndef MUSE_AGENT_CPP
#define MUSE_AGENT_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Meseret Gebre       gebremr@muohio.edu
//          Dhananjai M. Rao    raodm@muohio.edu
//
//---------------------------------------------------------------------------
#include "Agent.h"
#include "Simulation.h"
#include <iostream>
#include "f_heap.h"
using namespace std;
using namespace muse;

void
Agent::initialize() throw (std::exception) {}

void
Agent::executeTask(const EventContainer *events){}

void
Agent::finalize() {}

//-----------------remianing methods are defined by muse-----------
Agent::Agent(AgentID & id, State * agentState) : myID(id), LVT(0),myState(agentState) {}//end ctor

Agent::~Agent() {}

bool
Agent::processNextEvents(){
     //quick check, if eventpq empty, then return NULL
    if (eventPQ.empty()) return false;
    
    EventContainer events;
    Event *rootEvent = eventPQ.top();
    //cout << "Agent: " << getAgentID(); cout << " Event r time: " << rootEvent->getReceiveTime()<<endl;
    events.push_back(rootEvent); //add it to eventcontainer
    inputQueue.push_back(rootEvent); //push it to the input Queue
    eventPQ.pop(); //remove the root Event

    Event *nextEvent = NULL;
    if (eventPQ.size() > 1){ //means there is only one event
        nextEvent = eventPQ.top();
        ///now lets get all the events to process
        while(rootEvent->getReceiveTime() == nextEvent->getReceiveTime() ){
            if (eventPQ.empty()) break;
            events.push_back(nextEvent);
            inputQueue.push_back(nextEvent);
            eventPQ.pop();
            nextEvent = eventPQ.top();
        }//end while
    }//end if
    
    LVT = rootEvent->getReceiveTime(); //update the agents LVT
    executeTask(&events); //now call the executeTask
  
    State * state = myState->getClone(); //time to archive the agent's state
    state->timestamp=LVT;
    stateQueue.push_back(state);

    return true;
}//end processNextEvents

State*
Agent::cloneState(State * state){
    return state->getClone();
}//end cloneState

void
Agent::setState(State * state){
    //TODO: verify this works!!
    this->myState = state;
}//end setState

Time
Agent::getLVT() {return LVT;}

const 
AgentID& Agent::getAgentID(){
    return myID;
}//end getAgentID

bool 
Agent::scheduleEvent(Event *e){
    if (e->getReceiveTime() >(Simulation::getSimulator())->getEndTime() ) return false;
    //first check if this is a rollback!
    if (e->getReceiveTime() <= LVT){
        cout << "Detected a ROLLBACK" << endl;
        
    }
    if (e->getReceiverAgentID() == getAgentID()){
        e->increaseReference();
        eventPQ.push(e);
        e->increaseReference();
        outputQueue.push_back(e);
        return true;
    }else if ((Simulation::getSimulator())->scheduleEvent(e)){
        e->increaseReference();
        outputQueue.push_back(e);
        return true;
    }//end if
    return false;
}//end scheduleEvent

//bool
//Agent::scheduleEvents(EventContainer * events){
//    if ((Simulation::getSimulator()).scheduleEvents(events)){
//        //this means event was scheduled with no problems
//        //now lets add this event to our outputQueue in case of rollback
//        list<Event*>::iterator it = _outputQueue.end();
//        this->_outputQueue.insert(it, events->begin(),events->end());
//        return true;
//    }
//    return false;
//}

#endif
