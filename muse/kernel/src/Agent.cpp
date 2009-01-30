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
    if (rootEvent == NULL){
      cout << "trying to process a NULL event: returning"<<endl;
      eventPQ.pop();
      return false;
    }
    //cout << "Agent: " << getAgentID(); cout << " Event r time: " << rootEvent->getReceiveTime()<<endl;
    events.push_back(rootEvent); //add it to eventcontainer
    //rootEvent->increaseReference();
    inputQueue.push_back(rootEvent); //push it to the input Queue
    eventPQ.pop(); //remove the root Event

    Event *nextEvent = NULL;
    if (eventPQ.size() > 1){ //means there is only one event
        nextEvent = eventPQ.top();
        ///now lets get all the events to process
        while(rootEvent->getReceiveTime() == nextEvent->getReceiveTime() ){
            if (eventPQ.empty()) break;
            events.push_back(nextEvent);
            //nextEvent->increaseReference();
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
    if (e->getReceiveTime() >(Simulation::getSimulator())->getEndTime() ) return false;
    if (e == NULL) {
        cout << "Detected a NULL event" <<endl;
        return false;
    }
    // if (e->getSign() == false){
    //  cout << "Agent ["<<getAgentID<<"]"; cout<< e << "  :Detected an Anti-Message from agent: "<< e->getSenderAgentID()<<endl;
    //}
    //first check if this is a rollback!
    if (e->getReceiveTime() <= LVT){
        cout << "\nDetected a ROLLBACK @ agent: "<<getAgentID() << endl;
        cout << "Step 1. Find the state before the straggler time: "<< e->getReceiveTime() << endl;
        Time straggler_time = e->getReceiveTime();
        list<State*>::reverse_iterator it;
        cout << "   Searching";
        for(it=stateQueue.rbegin(); it != stateQueue.rend(); ++it)
        {
            cout << ".";
            if ((*it)->getTimeStamp() < straggler_time){
               setState((*it));
               cout << "    State Found for time: "<< (*it)->getTimeStamp() << endl;
               break;
            }//end if
        }//end for

        
        Time rollback_time = LVT = myState->getTimeStamp();
        cout << "\nStep 2. Send Anit-messages and prun Output Queue for events with time > "<< rollback_time<< endl;
        list<Event*>::iterator outQ_it = outputQueue.begin();
        AgentID current_agent = -1u;
        for(; outQ_it != outputQueue.end(); ++outQ_it)
        {
	    Event *current_event = (*outQ_it);
            //cout << "   current agent: "<< current_agent <<endl;
            if (current_event != NULL && current_event->getReceiverAgentID() != myID){ //means we can check the event


	      if (current_event->getSentTime() > rollback_time ){ //make sure that events with time > RBT are processed


		if (current_event->getReceiverAgentID() != current_agent){ //set bitVectors here.


		  current_agent = current_event->getReceiverAgentID();//wrong
                        current_event->makeAntiMessage();
                        scheduleEvent(current_event);
                        cout << "   Agent [ "<<getAgentID<<" ]";cout << " Sent Anti-Message to agent: " <<current_event->getReceiverAgentID();
                        cout << " with receive time: " <<current_event->getReceiveTime()  << endl;

                    }//end if


                    //cout << "   Anti-message ["<<(*outQ_it) << "] with ref count: " << (*outQ_it)->getReferenceCount() << endl;
                    current_event->deleteEvent();
                    current_event = NULL;
                    //cout << "  Event should be NULL: "<<(*outQ_it)->receiveTime <<endl;
                }//end outQ prun check

            }
        }//end for

        cout << "\nStep 3. Prunning input Queue for events with time > "<< rollback_time<< endl;
        //3. remove all events with time > rollback_time
        list<Event*>::iterator inQ_it = inputQueue.begin();
        cout << "Input Queue Size: " <<inputQueue.size() << endl;
        while ( inQ_it != inputQueue.end()) {
	  // if ((*inQ_it) != NULL){ //means we dont need to worry
	        list<Event*>::iterator del_it = intQ_it;
		intQ_it++;
	        
                if ( (*del_it)->getReceiveTime() > rollback_time && e->getSenderAgentID() == (*del_it)->getSenderAgentID()  ) {
                    cout << "Prunning Event: " <<(*del_it)<< " has sign: "<<(*del_it)->getSign() << " with ref count: " <<(*del_it)->getReferenceCount()<<endl;
                    (*del_it)->deleteEvent();
		    // If you erase intQ_it then the iterator is invalidated. 
		    // So save a reference to intQ_it, move intQ_it to the next entry 
		    // and then delete intQ_it
		  
                    // (*inQ_it)=NULL;
                }//end if
		else{
		  eventPQ.push( (*del_it) );
		}
		inputQueue.erase(del_it);
		// }//end if
        }//end for 

        cout << "Rollback Recovery Complete\n"<<endl;
	// std::exit(1);
    }//end rollback check

    if (e->getSign()) {
      eventPQ.push(e);
    }

    //check if event is an anti-message for the future
    if (e->getSign() == false && e->getReceiveTime() > LVT) {
        //we have to remove it from eventPQ
         //cout << "Detected Anti-message for the future to agent: "<< e->getReceiverAgentID()<<endl;
    }
    
    if (e->getReceiverAgentID() == getAgentID()){
        //cout << "Sending to own self"<<endl;
        eventPQ.push(e);
        //e->increaseReference();
        outputQueue.push_back(e);
       // cout << "ref count: " << e->referenceCount <<endl;
        return true;
    }else if ((Simulation::getSimulator())->scheduleEvent(e)){
      //e->increaseReference();
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
