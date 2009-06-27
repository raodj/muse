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
#include "HashMap.h"
#include "BinaryHeapWrapper.h"

#include <iostream>
#include <cstdlib>

using namespace std;
using namespace muse;

Agent::Agent(AgentID  id, State* agentState)
    : myID(id), lvt(0), myState(agentState), num_rollbacks(0),
      num_scheduled_events(0), num_processed_events(0), num_mpi_messages(0),
      numCommittedEvents(0) {
    // Make an Event Priority Queue
    eventPQ = new BinaryHeapWrapper();
}

Agent::~Agent() {
    // Let's make sure we dont have any left over events because
    // finalize() method should have properly cleaned up the Priority
    // Queue
    ASSERT(eventPQ->empty());
    
    delete eventPQ;
    delete myState;
}

bool
Agent::processNextEvents() {
    // The event Queue cannot be empty
    ASSERT(!eventPQ->empty());
    const Time gvt = getTime(GVT);
    if (getLVT() - gvt > 1209600000) {
        return false;
    }
    // create the event container.  this will be passed on to the
    // agent's executeTask method.
    EventContainer next_events;
    getNextEvents(next_events);
    // There *has* to be a next event
    ASSERT(!next_events.empty());
    
    //here we set the agent's LVT and update agent's state timestamp
    setLVT(next_events.front()->getReceiveTime());
    getState()->timestamp = getLVT();
    executeTask(&next_events);
    
    //keep track number of processed events
    num_processed_events += next_events.size();
    
    // Clear the references for the events in the container and clear it
    for (size_t i = 0; (i < next_events.size()); i++) {
        next_events[i]->decreaseReference();
    }
    next_events.clear();
    
    //clone the state so we can archive
    State* state = cloneState(getState());
    state->timestamp = getLVT();
    
    // There should not be a duplicate state in the queue
    ASSERT(!TIME_EQUALS(stateQueue.back()->getTimeStamp(),
                        state->getTimeStamp()));
    // The states should be monotomically increasing in timestamp
    ASSERT(stateQueue.back()->getTimeStamp() < state->getTimeStamp());
    // Save current state
    stateQueue.push_back(state);
    
    // Save the states of all of the SimStreams
    oss.saveState(getLVT());
    for (size_t i = 0; (i < allSimStreams.size()); i++){
        allSimStreams[i]->saveState(getLVT());
    }
    
    return true;
}

void
Agent::getNextEvents(EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(eventPQ->top() != NULL);
    const Time currTime = eventPQ->top()->getReceiveTime();
    do {
        Event* event = eventPQ->top();
        eventPQ->pop();
        
        //we should never process an anti-message.
        if (event->isAntiMessage()) { 
            cerr << "Anti-message Processing: " << *event << endl;
            cerr << "Trying to process an anti-message event, "
                 << "please notify MUSE developers of this issue" << endl;
            abort();
        }
    
        // Ensure that the top event is greater than LVT
        if (event->getReceiveTime() <= getLVT()) {
            cerr << "Agent is being scheduled to process an event ("
                 << *event << ") that is at or below it LVT (LVT="
                 << getLVT() << ", GVT=" << getTime(GVT)
                 << "). This is a serious error. Aborting.\n";
            cerr << *this << endl;
            abort();
        }

        // We add the top event we popped to the event container
        container.push_back(event);
    
        // Increase reference count, so we can add it to the agent's
        // input queue
        event->increaseReference(); 
        inputQueue.push_back(event);
                
    } while ((!eventPQ->empty()) &&
             (TIME_EQUALS(eventPQ->top()->getReceiveTime(), currTime)));   
}

State*
Agent::cloneState(State * state){
    return state->getClone();
}

void
Agent::setState(State * state){
    myState = state;
}

void
Agent::registerSimStream(SimStream * theSimStream){
    allSimStreams.push_back(theSimStream);
}

bool 
Agent::scheduleEvent(Event *e) {
    // Perform some sanity checks.
    ASSERT(TIME_EQUALS(e->getSentTime(), TIME_INFINITY));
    ASSERT(e->getSenderAgentID() == -1);
    ASSERT(e->isAntiMessage() == false);
    ASSERT(e->getReferenceCount() == 1);
    
    // Fill in the sent time and sender agent id info.
    e->sentTime      = getLVT();
    e->senderAgentID = getAgentID();
    
    // Check to make sure we don't schedule past the simulation end time.
    if (e->getSentTime() >= Simulation::getSimulator()->getStopTime()) {
        e->decreaseReference();
        return false;
    }
    
    // Make sure we don't schedule an event with a receive time that is
    // less than or equal to our LVT
    if (e->getReceiveTime() <= getLVT()){
        cerr << "You are trying to schedule an event with a smaller or equal "
             << "timestamp to your LVT, this is impossible and will cause "
             << "a rollback." << endl;
        e->decreaseReference();
        abort();
    }
    
    ASSERT(e->getReceiveTime() >= getTime(GVT));
    
    // Check and short-circuit scheduling to ourselves.
    if (e->getReceiverAgentID() == getAgentID()) {
        Time old_top_time = getTopTime();
        
        // Add directly to the event queue - there is no need to go
        // through the Scheduler.
        eventPQ->push(e);
        
        // Make sure the heap is still valid.
        Simulation::getSimulator()->updateKey(fibHeapPtr, old_top_time);
         
        // Add the event to the to output queue
        e->increaseReference();
        outputQueue.push_back(e);

        // Keep track of event being scheduled.
        num_scheduled_events++;
        
        return true;
    } else if (Simulation::getSimulator()->scheduleEvent(e)) {
        // Now that the event is in the Scheduler, update the output queue.
        if (Simulation::getSimulator()->isAgentLocal(e->getReceiverAgentID())) {
            e->increaseReference();
        }
        outputQueue.push_back(e);
       
        // Keep track of event being scheduled.
        num_scheduled_events++;

        // this is to keep track of how many MPI message we use
        if (!Simulation::getSimulator()->isAgentLocal(e->getReceiverAgentID())) {
            num_mpi_messages++;
        }
        return true;
    }

    // If control drops here, the event was rejected from
    // scheduling. Clean up.
    e->decreaseReference();
    return false;
}

void
Agent::doRollbackRecovery(const Event* straggler_event){
    // std::cerr << "Rolling back due to: " << *straggler_event << std::endl;
    //cout << "Rollback recovery started"<< endl;
    doRestorationPhase(straggler_event->getReceiveTime());
    //After state is restored, that means out current time is the restored time!
    Time restored_time = getTime(LVT);
    
    //std::cerr << "*** Agent(" << myID << "): restored time to "
    //          << restored_time << ", while GVT = " << getTime(GVT)
    //          << std::endl;
   
    doCancellationPhaseInputQueue(restored_time,
                                  straggler_event->getSenderAgentID());
    doCancellationPhaseOutputQueue(restored_time);
    
    //we need to rollback all SimStreams here.
    oss.rollback(restored_time);
    for (size_t i=0; (i < allSimStreams.size()); i++){
        allSimStreams[i]->rollback(restored_time);
    }

    //lets keep track of number of rollbacks
    num_rollbacks++;
  
}//end doRollback

void
Agent::doRestorationPhase(const Time& straggler_time){
   
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
        setLVT( getState()->getTimeStamp() ) ;
        //std::cout << "Restored Time To: " << getLVT() << std::endl;
    }else{
        //there is a problem if this happens
        //cout << "straggler time: " <<straggler_event->getReceiveTime() <<endl;
        //cout << "restored  time: " <<LVT <<endl;
        ASSERT( straggler_time > getLVT() );
    }
    
    //for debugging reasons
    //if (stateQueue.size() <= 2) cerr << "StateQ After Restore: "
    //                                << *this << endl;
                                    //<< " GVT: " << getTime(GVT)
                                    //<< " Straggler Time: "<< straggler_time
                                    //<< " Restored Time: "<< getTime() << endl;
   
}//end doStepOne

void
Agent::doCancellationPhaseOutputQueue(const Time & restored_time ){
  
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
        if (current_event->getSentTime() > restored_time) {
            //check if bitMap to receiver agent has been set.
            if (current_event->getReceiverAgentID() != getAgentID() &&
                bitMap[current_event->getReceiverAgentID()] == false) {
                bitMap[current_event->getReceiverAgentID()] = true;
                if (!(Simulation::getSimulator())->isAgentLocal(current_event->getReceiverAgentID()) ){
                    current_event->makeAntiMessage();
                    //cout << "Making Anti-Message: " << *current_event << endl;
                    Simulation::getSimulator()->scheduleEvent(current_event);
                }else{
                    //here we have to make a fresh new copy of the
                    //event and make the copy an anti-message. This
                    //clears a lot of issues.
                    Event*  anti_event = new Event(*current_event);
                    anti_event->setReferenceCount(1);
                    anti_event->makeAntiMessage();
                    Simulation::getSimulator()->scheduleEvent(anti_event);
                }
            }
            //invalid events automatically get removed
            current_event->decreaseReference();
            outputQueue.erase(del_it);
        }
    }
}//end doStepTwo

void
Agent::doCancellationPhaseInputQueue(const Time & restored_time, const AgentID & straggler_sender_agent_id ){
   
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
        ASSERT(current_event->isAntiMessage() == false );
        
        if ( current_event->getReceiveTime() > restored_time){
            //check if we need to re-process the current_event
            if( straggler_sender_agent_id  != current_event->getSenderAgentID() ){
                current_event->increaseReference();
                eventPQ->push(current_event );
                //std::cerr << "Moved from inputQueue to eventPQ: "
                //          << *current_event << std::endl;
            } else {
                // std::cerr << "Removed from inputQueue: "
                //         << *current_event << std::endl;
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

        //keep track number of processed events
        num_processed_events++;
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

    list<State*>::iterator safe_point_it = stateQueue.begin();
    Time one_below_gvt = gvt;

    
    one_below_gvt = 0;
    while (safe_point_it != stateQueue.end() && (*safe_point_it)->getTimeStamp() <= gvt ) {
        one_below_gvt = (*safe_point_it)->getTimeStamp();
        safe_point_it++;
    }
    
    //cerr << "Collecting Garbage now.....one_below_GVT: " <<
    //one_below_gvt <<" real GVT: "<<getTime(GVT) << "\n"; cerr <<
    //"States being collected for agent ("<<getAgentID()<<") are:
    //\n";
    
    //now we start looking
    while(stateQueue.front()->getTimeStamp() < one_below_gvt) {
        State *current_state = stateQueue.front();
        //cerr << "State @ time: " << current_state->getTimeStamp()<<"\n";
        delete current_state;
        stateQueue.pop_front();
    }
    //cerr << *this << endl;
    
    //second we collect from the inputQueue
    while(!inputQueue.empty() &&
          inputQueue.front()->getReceiveTime() < one_below_gvt){
        Event *current_event = inputQueue.front();
        current_event->decreaseReference();
        inputQueue.pop_front();
        
        //keep track number of processed events
        numCommittedEvents++;
    }
    
    //last we collect from the outputQueue
    while(!outputQueue.empty() &&
          outputQueue.front()->getSentTime() < one_below_gvt){
        Event *current_event = outputQueue.front();
        current_event->decreaseReference();
        outputQueue.pop_front();
    }
    
    
    //we need to garbageCollect all SimStreams here.
    oss.garbageCollect(one_below_gvt);
    for (size_t i = 0; (i < allSimStreams.size()); i++){
        allSimStreams[i]->garbageCollect(one_below_gvt);
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

Time
Agent::getTopTime() const {
    return eventPQ->empty() ? TIME_INFINITY : eventPQ->top()->getReceiveTime();
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
    if ( !agent.eventPQ->empty() ){
        os << "Agent[id="           << agent.getAgentID()    << ","
           << "# of Events in heap="<< agent.eventPQ->size() << ","
           << "top "                << *agent.eventPQ->top() << ","
           << "StateQueue: ("       << statePrinter(os,agent.stateQueue); //<< ")]" ;
    }else{
        os << "Agent[id="           << agent.getAgentID()    << ","
           << "top EVENT:"          << "EMPTY"               << ","
           << "StateQueue: ("       << statePrinter(os,agent.stateQueue); //<< ")]" ;
    }
    return os;
}



#endif
 
