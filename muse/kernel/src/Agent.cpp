
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
Agent::Agent(AgentID  id, State * agentState) : myID(id), LVT(0),myState(agentState) {
    eventPQ = new EventPQ();
}//end ctor

Agent::~Agent() {}

bool
Agent::processNextEvents(){
    
    if (eventPQ.empty()) {
      //cout << "eventPQ is empty: returning false"<<endl;
      return false;
    }
    
    EventContainer events;
    Event *top_event = eventPQ.top();eventPQ.pop();
    
    
    if ( top_event->isAntiMessage()) {
        cout << "eventPQ topped an Anti-Message: Ignoring and returning"<<endl;
        eventPQ.pop();
        return false;
    }
    
    ASSERT(top_event->getSenderAgentID() >= 0 && top_event ->getSenderAgentID() < 3 );
    ASSERT(top_event->getReceiverAgentID() >= 0 && top_event->getReceiverAgentID() < 3 );
    
    events.push_back(top_event);
    
    top_event->increaseReference(); 
    inputQueue.push_back(top_event);

    top_event->decreaseReference(); 
    while(eventPQ.size() != 0){
        Event *next_event = eventPQ.top();
        
        ASSERT(next_event->getSenderAgentID()   >= 0   && next_event->getSenderAgentID()    < 3 );
        ASSERT(next_event->getReceiverAgentID() >= 0   && next_event->getReceiverAgentID()  < 3 );
        
        if ( top_event->getReceiveTime() == next_event->getReceiveTime() ){
            eventPQ.pop();
            events.push_back(next_event);
            
            next_event->increaseReference(); 
            inputQueue.push_back(next_event);

            next_event->decreaseReference(); 
        }else{
            break;
        }
    }//end while
    
    myState->timestamp = LVT = top_event->getReceiveTime(); //update the agents LVT
    executeTask(&events); //now call the executeTask
    
    State * state = myState->getClone(); //time to archive the agent's state
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
    ASSERT(e->getSenderAgentID() >= 0 && e->getSenderAgentID() < 3 );
    ASSERT(e->getReceiverAgentID() >= 0 && e->getReceiverAgentID() < 3 );

    if ( e->getSentTime() >=(Simulation::getSimulator())->getEndTime() ){
        //dont bother to schedule because the simulation is done
        return false;
    }
    if (e->getReceiverAgentID() == getAgentID()){
      //cout << "Sending to own self"<<endl;
        e->increaseReference();
        eventPQ.push(e);
        e->increaseReference();
        outputQueue.push_back(e);
        //cout << "ref count: " << e->referenceCount <<endl;
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
    ASSERT(straggler_event->getReceiveTime() <= LVT);
    doStepOne(straggler_event);
    doStepTwo();
    doStepThree(straggler_event);
    ASSERT(straggler_event->getReceiveTime() > LVT  );
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
    ASSERT(straggler_event->getReceiveTime() > LVT  );
}//end doStepOne

void
Agent::doStepTwo(){
    Time rollback_time = myState->getTimeStamp();
    list<Event*>::iterator outQ_it = outputQueue.begin();
    map<AgentID, bool> bitMap;
    cout << "\nStep 2. Send Anit-messages and prun Output Queue for events with time > "<< rollback_time<< endl;
    cout << "          Output Queue Size: "<< outputQueue.size() <<endl;
    while(outQ_it != outputQueue.end()){
        list<Event*>::iterator del_it = outQ_it;
        outQ_it++; 
        Event *current_event = (*del_it);
      
        if ( current_event->getSentTime() > rollback_time ){ //means we can check the event
            if (current_event->getReceiverAgentID() != myID && bitMap[current_event->getReceiverAgentID()] == false ){ //check bitMap
                    bitMap[current_event->getReceiverAgentID()] = true;
                    current_event->makeAntiMessage();
                    scheduleEvent(current_event);
                    cout << "   Agent ["<<getAgentID()<<"]";cout << " Sent Anti-Message to agent: " <<current_event->getReceiverAgentID();
                    cout << " with receive time: " << current_event->getReceiveTime()  << endl;
            }//end if
            cout << " Prunning Event: " <<*current_event<<endl;
            current_event->decreaseReference();
            outputQueue.erase(del_it);
        }//end if outQ prun check
    }//end for
}//end doStepTwo

void
Agent::doStepThree(Event* straggler_event){
   Time rollback_time = myState->getTimeStamp();
   cout << "\nStep 3. Prunning input Queue for events with time > "<< rollback_time<< endl;
   cout << "          Input Queue Size: "<< inputQueue.size() <<endl;
   list<Event*>::iterator inQ_it = inputQueue.begin();
   while ( inQ_it != inputQueue.end()) {
     list<Event*>::iterator del_it = inQ_it;
     inQ_it++;
     Event *current_event = (*del_it);
     ASSERT(current_event->getSenderAgentID()   >= 0 && current_event->getSenderAgentID()   < 3 );
     ASSERT(current_event->getReceiverAgentID() >= 0 && current_event->getReceiverAgentID() < 3 );
    
     if ( current_event->getReceiveTime() > rollback_time){
         cout << " Prunning Event: " <<*current_event<<endl;
	 if( straggler_event->getSenderAgentID() != current_event->getSenderAgentID()){
             current_event->increaseReference();
             eventPQ.push(current_event );
	 }
         
         current_event->decreaseReference();
	 inputQueue.erase(del_it);
     }//end if
   }//end for
}//end doStepThree

void
Agent::cleanStateQueue(){
  //lets take care of all the states still not removed
  list<State*>::iterator state_it = stateQueue.begin();
  for (; state_it != stateQueue.end(); ++state_it ){
    delete (*state_it);
  }//end for
  //delete myState;
}//end cleanStateQueue

void
Agent::cleanInputQueue(){
  //lets take care of all the events in the inputQueue aka processed Events
  list<Event*>::iterator inQ_it = inputQueue.begin();
  cout << "Starting inQ deletion" <<endl;
  while ( inQ_it != inputQueue.end()) {
    list<Event*>::iterator del_it = inQ_it;
    inQ_it++;
    // cout << "Cleaning inQ: " << (*del_it) <<endl;
    (*del_it)->decreaseReference();
    inputQueue.erase(del_it);
  }//end for
}//end cleanInputQueue

void
Agent::cleanOutputQueue(){
//now lets delete all remaining events in each agent's outputQueue
  list<Event*>::iterator outQ_it = outputQueue.begin();
  cout << "Starting outQ deletion" <<endl;
  while(outQ_it != outputQueue.end()){
    list<Event*>::iterator del_it = outQ_it;
    outQ_it++;
    (*del_it)->decreaseReference();
    outputQueue.erase(del_it);
  }//end while
}//end cleanOutputQueue


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

bool
Agent::agentComp::operator()(const Agent *lhs, const Agent *rhs) const
{
  //cout << "Here" <<endl;
  if (lhs->eventPQ.empty() && rhs->eventPQ.empty()) {
    return true;
  }
  if (lhs->eventPQ.empty() && !rhs->eventPQ.empty()) {
    return true;
  }
  if (rhs->eventPQ.empty() && !lhs->eventPQ.empty()) {
    return false;
  }
  Event *lhs_event = lhs->eventPQ.top();
  Event *rhs_event = rhs->eventPQ.top();
  Time lhs_time    = lhs_event->getReceiveTime();
  Time rhs_time    = rhs_event->getReceiveTime();
  return (lhs_time > rhs_time);
}//end operator()

#endif
