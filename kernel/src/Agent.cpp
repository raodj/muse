
#include "Event.h"

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
#include <map>
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
    if ( eventPQ.top()->getSign() == false ) {
        cout << "eventPQ topped an Anti-Message: Ignoring and returning"<<endl;
        eventPQ.pop();
        return false;
    }

    EventContainer events;
    Event *rootEvent = eventPQ.top();
    if (rootEvent == NULL){
      cout << "trying to process a NULL event: returning"<<endl;
      eventPQ.pop();
      return false;
    }
    //cout << "Agent: " << getAgentID(); cout << " Event r time: " << rootEvent->getReceiveTime()<<endl;
    events.push_back(rootEvent); //add it to eventcontainer
    rootEvent->increaseReference();
    inputQueue.push_back(rootEvent); //push it to the input Queue
    eventPQ.pop(); //remove the root Event

    Event *nextEvent = NULL;
    if (eventPQ.size() > 1){ //means there is only one event
        nextEvent = eventPQ.top();
        ///now lets get all the events to process
        while(rootEvent->getReceiveTime() == nextEvent->getReceiveTime() ){
            if (eventPQ.empty()) break;
            events.push_back(nextEvent);
            nextEvent->increaseReference();
            inputQueue.push_back(nextEvent);
            eventPQ.pop();
            nextEvent = eventPQ.top();
        }//end while
    }//end if
    
    LVT = rootEvent->getReceiveTime(); //update the agents LVT
    executeTask(&events); //now call the executeTask
    myState->timestamp = LVT;
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

    //first check if this is a rollback!
    if (e->getReceiveTime() <= LVT){
        cout << "\nDetected a ROLLBACK @ agent: "<<getAgentID() << endl <<endl;
        doRollbackRecovery(e);
        cout << "Rollback Recovery Complete\n"<<endl;
	//std::exit(1);
    }//end rollback check

    //check if event is an anti-message for the future
    if (e->getSign() == false && e->getReceiveTime() > LVT) {
        //we must re it and its subtree aways
        cout << "-Detected Anti-message for the future to agent: "<< e->getReceiverAgentID()<<endl;
        EventPQ::iterator it = eventPQ.begin();
        for (; it != eventPQ.end(); it++){
            if ( (*it)->getReceiveTime() >= e->getReceiveTime() &&
                 (*it)->senderAgentID == e->getSenderAgentID()){
                (*it)->makeAntiMessage();
                cout << "---Tagged Anti-message"<<endl;
                //eventPQ.remove(it.getPointer());
            }//end if
        }//end for
        return true;
    }
    
    if (e->getReceiverAgentID() == getAgentID()){
        //cout << "Sending to own self"<<endl;
        eventPQ.push(e);
        e->increaseReference();
        outputQueue.push_back(e);
       // cout << "ref count: " << e->referenceCount <<endl;
        return true;
    }else if ((Simulation::getSimulator())->scheduleEvent(e)){
        e->increaseReference();
        outputQueue.push_back(e);
        return true;
    }//end if
    return false;
}//end scheduleEvent

void
Agent::doRollbackRecovery(Event* straggler_event){
    doStepOne(straggler_event);
    doStepTwo();
    doStepThree(straggler_event);
}//end doRollback

void
Agent::doStepOne(Event* straggler_event){
    cout << "Step 1. Find the state before the straggler time: "<< straggler_event->getReceiveTime() << endl;
    Time straggler_time = straggler_event->getReceiveTime();
    list<State*>::reverse_iterator it;
    cout << "   Searching";
    for(it=stateQueue.rbegin(); it != stateQueue.rend(); ++it){
        cout << ".";
        if ((*it)->getTimeStamp() < straggler_time){
           setState((*it));
           LVT = myState->getTimeStamp();
           cout << "    State Found for time: "<< (*it)->getTimeStamp() << endl;
           break;
        }//end if
    }//end for
}//end doStepOne

void
Agent::doStepTwo(){
    Time rollback_time = myState->getTimeStamp();
    list<Event*>::iterator outQ_it = outputQueue.begin();
    map<AgentID, bool> bitMap;
    cout << "\nStep 2. Send Anit-messages and prun Output Queue for events with time > "<< rollback_time<< endl;

    while(outQ_it != outputQueue.end()){
        list<Event*>::iterator del_it = outQ_it;
        outQ_it++;
        Event *current_event = (*del_it);
        //cout << "   current agent: "<< current_agent <<endl;
        if (current_event->getReceiverAgentID() != myID &&
            current_event->getSentTime() > rollback_time ){ //means we can check the event

            if ( bitMap[current_event->getReceiverAgentID()] == false ){ //check bitMap
                    bitMap[current_event->getReceiverAgentID()] = true;
                    current_event->makeAntiMessage();
                    scheduleEvent(current_event);
                    cout << "   Agent ["<<getAgentID()<<"]";cout << " Sent Anti-Message to agent: " <<current_event->getReceiverAgentID();
                    cout << " with receive time: " <<current_event->getReceiveTime()  << endl;
            }//end if
            current_event->decreaseReference();
            outputQueue.erase(del_it);
        }//end if outQ prun check
    }//end for
}//end doStepTwo

void
Agent::doStepThree(Event* straggler_event){
    Time rollback_time = myState->getTimeStamp();
    cout << "\nStep 3. Prunning input Queue for events with time > "<< rollback_time<< endl;
    list<Event*>::iterator inQ_it = inputQueue.begin();
    while ( inQ_it != inputQueue.end()) {
            list<Event*>::iterator del_it = inQ_it;
            inQ_it++;
            if ( (*del_it)->getReceiveTime() > rollback_time &&
                 straggler_event->getSenderAgentID() != (*del_it)->getSenderAgentID()){
                    cout << " Prunning Event: " <<(*del_it)<< " has sign: "<<(*del_it)->getSign() << " with ref count: " <<(*del_it)->getReferenceCount()<<endl;
                    eventPQ.push( (*del_it) );
            }//end if
            
            (*del_it)->decreaseReference();
            inputQueue.erase(del_it);
    }//end for
}//end doStepThree

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
