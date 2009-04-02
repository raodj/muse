
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
#include "HashMap.h"

#include <cstdlib>

using namespace std;
using namespace muse;



void
Agent::initialize() throw (std::exception) {}

void
Agent::executeTask(const EventContainer *events){}

void
Agent::finalize() {}

//-----------------remianing methods are defined by muse-----------
Agent::Agent(AgentID  id, State * agentState) : myID(id),myState(agentState),lvt(0){
    eventPQ = new EventPQ;
}//end ctor

Agent::~Agent() {
    //lets make sure we dont have any left over events
    while(!eventPQ->empty()){
        eventPQ->top()->decreaseReference();
        eventPQ->pop();
    }
    delete eventPQ;
}

bool
Agent::processNextEvents(){
    //cout <<getAgentID();cout << " eventPQ size: " <<eventPQ->size() <<endl;
    //here we make sure the list is not empty
    ASSERT(eventPQ->empty() != true);
    if (eventPQ->empty()) {
      return false;
    }

    //create the event container.  this will be passed on to the
    //agent's executeTask method.
    EventContainer events;
    Event *top_event = eventPQ->top();eventPQ->pop();

    
    //if (getAgentID() == 0) cout << "NEW LVT: " <<top_event->getReceiveTime() <<endl;
    //we should never process an anti-message.
    if (top_event->isAntiMessage() ){
        cerr.flush();
        cerr<<"Processing: " << *top_event<<endl;
        cerr.flush();
        cerr << "Trying to process an anti-message event, please notify MUSE developers of this issue" << endl;
        cerr.flush();
        abort();
        /// ASSERT(top_event->isAntiMessage() == false );
    }
   
    
   
    //we add the top event we popped to the event container
    events.push_back(top_event);

    //increase reference count, so we can add it to the agent's input queue
    top_event->increaseReference(); 
    inputQueue.push_back(top_event);

    //since we popped an event from the eventPQ, we must call to
    //decrease the reference.
    top_event->decreaseReference();

    //this while is used to gather the remaining event that will be
    //processed
    while(eventPQ->size() != 0){
        //first top the next event for checking.
        Event *next_event = eventPQ->top();
        
        //if the receive times match, then they are to be processed at
        //the same time.
        if ( TIME_EQUALS( top_event->getReceiveTime() , next_event->getReceiveTime()) ){
        //if ( top_event->getReceiveTime() == next_event->getReceiveTime() ){
            //first remove it from the eventPQ
            eventPQ->pop();
            //now we add it to the event container
            events.push_back(next_event);
            //increase the reference count, since it will be added to
            //the input queue.
            next_event->increaseReference();
            inputQueue.push_back(next_event);
            //finally, we decrease the reference count for the pop.
            next_event->decreaseReference(); 
        }else{
            break;
        }
    }//end while

   
    //here we set the agent's LVT and update agent's state timestamp
    setLVT(top_event->getReceiveTime());
    getState()->timestamp = getLVT();
    executeTask(&events);
    
    //clone the state so we can archive
    State * state = cloneState(getState());
    
    //lets inspect the last state in the queue and make sure the next
    //state to be push has a bigger timestep
    //if (getAgentID() == 0)cout << "stateQueue.back()->getTimeStamp() === " << stateQueue.back()->getTimeStamp() <<endl;
    //if (getAgentID() == 0)cout << "state->getTimeStamp()             === " << state->getTimeStamp() <<endl;
    //after the second state in the stateQueue, there should never be a duplicate again
    if ( stateQueue.size() > 2 ) {
        ASSERT( !TIME_EQUALS(stateQueue.back()->getTimeStamp(),state->getTimeStamp()) );
        ASSERT( stateQueue.back()->getTimeStamp() < state->getTimeStamp() );
    }

    stateQueue.push_back(state);

    //we finally need to save the state of all SimStreams that are registered.
    oss.saveState(getLVT());
    for (int i=0;i<allSimStreams.size(); i++){
        allSimStreams[i]->saveState(getLVT());
    }
    return true;
}//end processNextEvents

State*
Agent::cloneState(State * state){
    return state->getClone();
}//end cloneState

void
Agent::setState(State * state){
    myState = state;
}//end setState

void
Agent::registerSimStream(SimStream * theSimStream){
    allSimStreams.push_back(theSimStream);
}

bool 
Agent::scheduleEvent(Event *e){
    ASSERT(TIME_EQUALS(e->getSentTime(),TIME_INFINITY) );
    ASSERT(e->getSenderAgentID() == -1u);
    ASSERT(e->isAntiMessage() == false );
    
    //check to make sure that event scheduled via this method is
    //a new event
    if (!TIME_EQUALS(e->getSentTime(),TIME_INFINITY) || e->getSenderAgentID() != -1u ) {
        cerr << "Can't schedule this event it has already been scheduled and most likely to become a straggler event" <<endl;
        abort();
    }
    //fill in the sent time and sender agent id info
    e->sentTime      = getLVT();
    e->senderAgentID = getAgentID();
    
    //check to make sure we dont schedule pass the simulation end time.
    if ( e->getSentTime() >= (Simulation::getSimulator())->getStopTime() ){   
        e->decreaseReference();
        return false;
    }

    
    //check to make sure we are not scheduling to one self.
    if (e->getReceiverAgentID() == getAgentID()){
        if (e->getReceiveTime() <= getLVT()){
            //this should NEVER happen. It's time to abort
            cerr << "You are trying to schedule an event to the same agent with a smaller "
                 << "timestamp, this is impossible and will cause a rollback" <<endl;
            abort();
        }
        
        //will use this to figure out if we need to change our key in
        //scheduler
        Time old_receive_time = (!eventPQ->empty()) ? eventPQ->top()->getReceiveTime() : TIME_INFINITY;
               
        //add to event scheduler this is a optimization trick, because
        //we dont go through the Simulation scheduler method.
        e->increaseReference(); 
        eventPQ->push(e);

        //now lets make sure that the heap is still valid
        //we have to change if the event receive time has a smaller key
        //then the top event in the heap.
        if ( e->getReceiveTime() < old_receive_time  ) {
            //we need to call for the key change
            (Simulation::getSimulator())->changeKey(fibHeapPtr,this);
        }
        
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

    //if it got to this point it was rejected for scheduling and we should release the memory
    e->decreaseReference();
    return false;
}//end scheduleEvent

void
Agent::doRollbackRecovery(Event* straggler_event){
    //cout << "Rollback recovery started"<< endl;
    doRestorationPhase(straggler_event);
    doCancellationPhaseOutputQueue();
    doCancellationPhaseInputQueue(straggler_event);

    //we need to rollback all SimStreams here.
    //LVT by now should be the restored_time
    oss.rollback(getLVT());
    for (int i=0;i<allSimStreams.size(); i++){
        allSimStreams[i]->rollback(getLVT());
    }
}//end doRollback

void
Agent::doRestorationPhase(Event* straggler_event){
    // cout << "Step 1. Find the state before the straggler time: "<< straggler_event->getReceiveTime() << endl;
    Time straggler_time = straggler_event->getReceiveTime();
    
    /** OK, here is the plan. First, there is a straggler_time.Second,
        the stateQueue should be sorted by the nature of its
        containt.We want to find a state with a timestamp smaller than
        the straggler_time.  Assuming the second reason is correct, if
        we search the stateQueue starting at the back, we not only
        find the state we want quicker, but we also find the time
        that's directly behind the straggler_time.  Since the
        stateQueue is a double linked list, we can by pass dealing
        with reverse_iterators and keep popping from the back until we
        find the state we want.If we fail to locate state with a
        timestamp that is older than straggler_time, then we revert
        back to the init_state that we got from the ctor.  The
        following code implements this idea!
    */

    //for debugging reasons
    //cout << "StateQueue B Restoration: ";
    //cout << *this << endl;

    //we set our LVT to INFINITY here incase we dont find a state to restore to
    //we can revert to the initial state after the loop.
    setLVT(TIME_INFINITY);
    //now we go and look for a state to restore to.
    while (stateQueue.size() != 1 ){
        State * current_state = stateQueue.back();
        if (current_state->getTimeStamp() < straggler_time){
             //first we destroy the state we are in now <--kind of wierd to say right?
            delete myState;
            //set out state to this old consistent state
            //after we setState, remember myState will point to a new state now.
            setState( cloneState(current_state) );
            //set agent's LVT to this state's timestamp
            setLVT(getState()->getTimeStamp());
            //cout << "    State Found for time: "<< LVT << endl;
            break;
        }else{
            //we should delete and remove this state from the list because it is no longer valid
            //cout << "Deleting Current_State @ timestamp: " << current_state->getTimeStamp() << endl;
            delete current_state;
            stateQueue.pop_back();
        }
    }
    
    //cout << "TIME_EQUALS(LVT,INFINITY) === " <<TIME_EQUALS(LVT,INFINITY) << endl;
    //if LVT is INFINITY then that means we have rolled back all the way to beginning.
    if (TIME_EQUALS(getLVT(),TIME_INFINITY) ){
        //state queue should have a size of one
        ASSERT(stateQueue.size() == 1);
        //cout << "TIME_EQUALS(LVT,INFINITY) = TRUE" << endl;
        delete myState;
        setState( cloneState(stateQueue.front()) );
        setLVT(getState()->getTimeStamp()) ;
        // cout << "Restored Time To: " << LVT <<endl;
    }else{
        //there is a problem if this happens
        //cout << "straggler time: " <<straggler_event->getReceiveTime() <<endl;
        //cout << "restored  time: " <<LVT <<endl;
        ASSERT(straggler_event->getReceiveTime() > getLVT()  );
    }
    
    //for debugging reasons
    //cout << "StateQueue A Restoration: ";
    //cout << *this << endl;

}//end doStepOne

void
Agent::doCancellationPhaseOutputQueue(){
    //cout << "Step 2. Send Anit-messages and remove from Output Queue for events with time > "<<myState->getTimeStamp() << endl;
    Time restored_time = getState()->getTimeStamp(); //TODO change name
   
    AgentIDBoolMap bitMap;
    
    /** OK, for step two here is what we are doing. First, we have the
        restored_time. This is the time we rollbacked to.  Second, is
        the bitMap, this is used for the anti-message optimization
        feature. Since we only need to send one anti-message of the
        earliest invalid event to each agent that needs the
        anti-message, we use the bitMap to mark if an agent has
        already received an anti-message. In this step we are checking
        the sent times all all the events in the outputQueue and those
        events with sent times greater than the restored_time are
        considered invalid events. If the receiver agentID is NOT
        equal to <this> agentID AND the bitMap is not set to TRUE,
        then we set it, and send the anti-message.  Lasly we remove
        all invalid events from the outputQueue. The following
        implements this idea!
     */
    list<Event*>::iterator outQ_it = outputQueue.begin();
    while(outQ_it != outputQueue.end()){
        list<Event*>::iterator del_it = outQ_it;
        outQ_it++; 
        Event *current_event = (*del_it);
        //check if the event is invalid.
        if ( current_event->getSentTime() > restored_time ){
            //check if bitMap to receiver agent has been set.
            if (current_event->getReceiverAgentID() != getAgentID() &&
                bitMap[current_event->getReceiverAgentID()] == false ){
                
                bitMap[current_event->getReceiverAgentID()] = true;
                current_event->makeAntiMessage();
                //cout << "Making Anti-Message: " << *current_event << endl;
                (Simulation::getSimulator())->scheduleEvent(current_event);
            }
            //invalid events automatically get removed
            current_event->decreaseReference();
            outputQueue.erase(del_it);
        }
    }
}//end doStepTwo

void
Agent::doCancellationPhaseInputQueue(Event* straggler_event){
    //cout << "Step 3. Remove from  input Queue for events with time > "<<myState->getTimeStamp() << endl;
    Time restored_time = getState()->getTimeStamp();

    /** OK, for step three, here is what we are doing. First, we have
        the restored_time. This is the time we rollbacked to.  This
        step is relatively straight forward. In this step, an invalid
        event is defined as and event with a receive time greater than
        the restored_time. Hence, all invalid events are automatically
        removed from the inputQueue. However, if the current_event's
        sender agent id is NOT EQUAL to the straggler_event's agent,
        then this means that we need to add it back into our event
        scheduler because we need to re-process this
        current_event. The following implements this idea!
    */
    list<Event*>::iterator inQ_it = inputQueue.begin();
    while ( inQ_it != inputQueue.end()) {
        list<Event*>::iterator del_it = inQ_it;
        inQ_it++;
        Event *current_event = (*del_it);

        //check if the event is invalid.
        if (current_event->isAntiMessage() ){
            //cerr.flush();
            //cerr <<"Go Anti Event @ step3: " <<*current_event <<endl;
            //cerr.flush();
            //invalid events automatically get removed
            current_event->decreaseReference();
            inputQueue.erase(del_it);
        }
        else if ( current_event->getReceiveTime() > restored_time){
            //check if we need to re-process the current_event
            if( straggler_event->getSenderAgentID() != current_event->getSenderAgentID() ){
                current_event->increaseReference();
                ASSERT(current_event->isAntiMessage() == false );
                eventPQ->push(current_event );
            }
            //invalid events automatically get removed
            current_event->decreaseReference();
            inputQueue.erase(del_it);
        }
    }
}//end doStepThree

void
Agent::cleanStateQueue(){
    //lets take care of all the states still not removed
    while(!stateQueue.empty() ){
        State *current_state = stateQueue.front();
        delete current_state;
        stateQueue.pop_front();
    }
}//end cleanStateQueue

void
Agent::cleanInputQueue(){
    //lets take care of all the events in the inputQueue
    while(!inputQueue.empty() ){
        Event *current_event = inputQueue.front();
        current_event->decreaseReference();
        inputQueue.pop_front();
    }
}//end cleanInputQueue

void
Agent::cleanOutputQueue(){
    //now lets delete all remaining events in each agent's outputQueue
    while(!outputQueue.empty() ){
        Event *current_event = outputQueue.front();
        current_event->decreaseReference();
        outputQueue.pop_front();
    }
}//end cleanOutputQueue

void
Agent::garbageCollect(const Time gvt){
    //cout << "Collecting Garbage now.....GVT: " << gvt <<endl;
    //first we collect from the stateQueue
    while(!stateQueue.empty() && stateQueue.front()->getTimeStamp() < gvt){
        State *current_state = stateQueue.front();
        delete current_state;
        stateQueue.pop_front();
    }
    
    //second we collect from the inputQueue
    while(!inputQueue.empty() &&  inputQueue.front()->getReceiveTime() < gvt){
        Event *current_event = inputQueue.front();
        current_event->decreaseReference();
        inputQueue.pop_front();
    }

    //last we collect from the outputQueue
    while(!outputQueue.empty() && outputQueue.front()->getSentTime() < gvt){
        Event *current_event = outputQueue.front();
        current_event->decreaseReference();
        outputQueue.pop_front();
    }

    //we need to garbageCollect all SimStreams here.
    oss.garbageCollect(gvt);
    for (int i=0;i<allSimStreams.size(); i++){
        allSimStreams[i]->garbageCollect(gvt);
    }
}

Time
Agent::getTime(TimeType time_type) const {
    switch(time_type){
    case LVT:
        return getLVT();
        break;
    case LGVT:
        return (Simulation::getSimulator())->getLGVT();
        break;
    case GVT:
        return (Simulation::getSimulator())->getGVT();
        break;
    }
    return TIME_INFINITY;
}

bool
Agent::agentComp::operator()(const Agent *lhs, const Agent *rhs) const
{
    Time lhs_time = lhs->eventPQ->empty() ? TIME_INFINITY : lhs->eventPQ->top()->getReceiveTime();
    Time rhs_time = rhs->eventPQ->empty() ? TIME_INFINITY : rhs->eventPQ->top()->getReceiveTime();

    //cout << "Comparing agents " << lhs->getAgentID()  << "(lhs_time = " << lhs_time
    // << ") and " << rhs->getAgentID() << " (rhs_time = " << rhs_time << ")\n";
    //if (lhs_time != TIME_INFINITY && rhs_time != TIME_INFINITY ) return (lhs_time > rhs_time);
    return (lhs_time >= rhs_time);
}

ostream&
statePrinter(ostream& os, list<muse::State*> state_q ){
    //list<muse::State*>::reverse_iterator rit = state_q.rbegin();
    //for (; rit != state_q.rend(); rit++){
    list<muse::State*>::iterator it = state_q.begin();
    for (; it != state_q.end(); it++){
        os << (*it)->getTimeStamp() << ",";
    }
    os << ")]    " ;
    return os;
}

ostream&
operator<<(ostream& os, const muse::Agent& agent) {
    os << "Agent[id="           << agent.getAgentID()   << ","
        //<< "top time="           << ((!agent.eventPQ->empty()) ? agent.eventPQ->top()->getReceiveTime():TIME_INFINITY) << ","
       << "StateQueue: ("       << statePrinter(os,agent.stateQueue); //<< ")]" ;
    return os;
}



#endif
 
