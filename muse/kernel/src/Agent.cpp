
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

    //here we make sure the list is not empty
    if (eventPQ->empty()) {
      return false;
    }

    //create the event container.
    //this will be passed on to the agent's executeTask method.
    EventContainer events;
    Event *top_event = eventPQ->top();eventPQ->pop();
    
    //we should never process an anti-message.
    ASSERT(top_event->isAntiMessage() == false );
    
    //we add the top event we popped to the event container
    events.push_back(top_event);

    //increase reference count, so we can add it to the agent's input queue
    top_event->increaseReference(); 
    inputQueue.push_back(top_event);

    //since we popped an event from the eventPQ, we must call to decrease the reference.
    top_event->decreaseReference();

    //this while is used to gather the remaining event that will be processed
    while(eventPQ->size() != 0){
        //first top the next event for checking.
        Event *next_event = eventPQ->top();
        //if the receive times match, then they are to be processed at the same time.
        if ( top_event->getReceiveTime() == next_event->getReceiveTime() ){
            //first remove it from the eventPQ
            eventPQ->pop();
            //now we add it to the event container
            events.push_back(next_event);
            //increase the reference count, since it will be added to the input queue.
            next_event->increaseReference();
            inputQueue.push_back(next_event);
            //finally, we decrease the reference count for the pop.
            next_event->decreaseReference(); 
        }else{
            break;
        }
    }//end while

    //here we set the agent's LVT and update agent's state timestamp
    myState->timestamp = LVT = top_event->getReceiveTime();
    executeTask(&events); 
    //clone the state so we can archive
    State * state = myState->getClone(); 
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


bool 
Agent::scheduleEvent(Event *e){
    //check to make sure we dont schedule pass the simulation end time.
    if ( e->getSentTime() >= (Simulation::getSimulator())->getEndTime() ){
        return false;
    }
    //check to make sure we are not scheduling to one self.
    if (e->getReceiverAgentID() == getAgentID()){
        //add to event scheduler
        //this is a optimization trick, because we dont go through the Simulation scheduler method.
        e->increaseReference();
        eventPQ->push(e);
        //add to output queue
        e->increaseReference();
        outputQueue.push_back(e);
        return true;
    }else if ((Simulation::getSimulator())->scheduleEvent(e)){
        //just add to output queue.
        e->increaseReference();
        outputQueue.push_back(e);
        return true;
    }//end if
    return false;
}//end scheduleEvent

void
Agent::doRollbackRecovery(Event* straggler_event){
    //the straggler event must have a smaller or equal time as the LVT
    ASSERT(straggler_event->getReceiveTime() <= LVT);
    doStepOne(straggler_event);
    doStepTwo(straggler_event);
    doStepThree(straggler_event);
}//end doRollback

void
Agent::doStepOne(Event* straggler_event){
    cout << "Step 1. Find the state before the straggler time: "<< straggler_event->getReceiveTime() << endl;
    //first we grab the straggler time
    Time straggler_time = straggler_event->getReceiveTime();
    //second we start looping in the reverse direction, because typically rollbacks happen towards the end of this queue.
    list<State*>::reverse_iterator it;
    cout << "   Searching";
    for(it=stateQueue.rbegin(); it != stateQueue.rend(); ++it){
        cout << ".";
        //this check is safe because the state queue is sorted ascending by timestamp
        if ((*it)->getTimeStamp() < straggler_time){
            //set out state to this old consistent state
            setState((*it));
            //set agent's LVT to this state's timestamp
            LVT = myState->getTimeStamp();
            cout << "    State Found for time: "<< (*it)->getTimeStamp() << endl;
            break;
        }
    }
    //at this point our LVT should be smaller then the straggle event's receive time
    ASSERT(straggler_event->getReceiveTime() > LVT  );
}//end doStepOne

void
Agent::doStepTwo(Event* straggler_event ){
    Time rollback_time = myState->getTimeStamp();
    //we make sure that doStepOne was called by this assert.
    ASSERT(straggler_event->getReceiveTime()  > rollback_time );
    list<Event*>::iterator outQ_it = outputQueue.begin();
    //this map is used to make sure we send only one anti-message to any given agent.
    map<AgentID, bool> bitMap;
    cout << "\nStep 2. Send Anit-messages and prun Output Queue for events with time > "<< rollback_time<< endl;
    cout << "          Output Queue Size: "<< outputQueue.size() <<endl;
    while(outQ_it != outputQueue.end()){
        list<Event*>::iterator del_it = outQ_it;
        outQ_it++; 
        Event *current_event = (*del_it);
        
        if ( current_event->getSentTime() > rollback_time ){
            
            if (current_event->getReceiverAgentID() != myID && bitMap[current_event->getReceiverAgentID()] == false ){ 
                bitMap[current_event->getReceiverAgentID()] = true;
                current_event->makeAntiMessage();
                scheduleEvent(current_event);
                cout << "   Agent ["<<getAgentID()<<"]";cout << " Sent Anti-Message to agent: " <<current_event->getReceiverAgentID();
                cout << " with receive time: " << current_event->getReceiveTime()  << endl;
            }
            
            cout << "   Prunning Event: " <<*current_event<<endl;
            current_event->decreaseReference();
            outputQueue.erase(del_it);
        }
    }
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
    
     if ( current_event->getReceiveTime() > rollback_time){
         cout << " Prunning Event: " <<*current_event<<endl;
	 if( straggler_event->getSenderAgentID() != current_event->getSenderAgentID()){
             current_event->increaseReference();
             eventPQ->push(current_event );
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


bool
Agent::agentComp::operator()(const Agent *lhs, const Agent *rhs) const
{
  
  if (lhs->eventPQ->empty() && rhs->eventPQ->empty()) {
    return true;
  }
  if (lhs->eventPQ->empty() && !rhs->eventPQ->empty()) {
    return true;
  }
  if (rhs->eventPQ->empty() && !lhs->eventPQ->empty()) {
    return false;
  }
  Event *lhs_event = lhs->eventPQ->top();
  Event *rhs_event = rhs->eventPQ->top();
  Time lhs_time    = lhs_event->getReceiveTime();
  Time rhs_time    = rhs_event->getReceiveTime();
  return (lhs_time > rhs_time);
}//end operator()

#endif
