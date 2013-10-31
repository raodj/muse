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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//          Alex Chernyakhovsky    alex@searums.org
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

Agent::Agent(AgentID id, State* agentState)
    : myID(id), lvt(0), myState(agentState), numRollbacks(0),
      numScheduledEvents(0), numProcessedEvents(0), numMPIMessages(0),
      numCommittedEvents(0) {
    // Make an Event Priority Queue
    eventPQ = new BinaryHeapWrapper();
}

Agent::~Agent() {
    // Let's make sure we dont have any left over events because
    // finalize() method should have properly cleaned up the Priority
    // Queue
    ASSERT(eventPQ->empty());
    ASSERT(inputQueue.empty());
    ASSERT(outputQueue.empty());
    ASSERT(stateQueue.empty());
    
    delete eventPQ;
    delete myState;
}

void
Agent::saveState() {
    // Clone the current state so it can be saved
    State* state = cloneState(getState());
    state->timestamp = getLVT();
    
    // The states should be monotomically increasing in timestamp
    ASSERT(stateQueue.empty() ||
           (stateQueue.back()->getTimeStamp() < state->getTimeStamp()));
    // Save current state
    stateQueue.push_back(state);
    
    // Save the states of all of the SimStreams
    oss.saveState(getLVT());
    for (size_t i = 0; (i < allSimStreams.size()); i++) {
        allSimStreams[i]->saveState(getLVT());
    }
}

bool
Agent::processNextEvents() {
    // The event Queue cannot be empty
    ASSERT(!eventPQ->empty());
    const Time gvt = getTime(GVT);
    if (getLVT() - gvt > 604800000) {
        return false;
    }
    // Create the event container. This will be passed on to the
    // agent's executeTask method.
    EventContainer nextEvents;
    getNextEvents(nextEvents);
    // There *has* to be a next event
    ASSERT(!nextEvents.empty());
    
    // Set the LVT and timestamp
    setLVT(nextEvents.front()->getReceiveTime());
    getState()->timestamp = getLVT();
    executeTask(&nextEvents);
    
    // Increment the numProcessedEvents counter
    numProcessedEvents += nextEvents.size();

    // Save the state now that events have been processed.
    saveState();
    
    return true;
}

void
Agent::getNextEvents(EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(eventPQ->top() != NULL);
    const Time currTime = eventPQ->top()->getReceiveTime();
    do {
        Event* event = eventPQ->top();
        // We should never process an anti-message.
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

        ASSERT(event->getReferenceCount() < 3);
        
        // We add the top event we popped to the event container
        container.push_back(event);

        DEBUG(std::cout << "Delivering: " << *event << std::endl);
        
        // Increase reference count, so we can add it to the agent's
        // input queue
        event->increaseReference(); 
        inputQueue.push_back(event);

        // Finally it is safe to remove this event from the eventPQ as
        // it has been added to the inputQueue
        eventPQ->pop();
    } while ((!eventPQ->empty()) &&
             (TIME_EQUALS(eventPQ->top()->getReceiveTime(), currTime)));   
}

State*
Agent::cloneState(State* state) {
    return state->getClone();
}

void
Agent::setState(State* state) {
    myState = state;
}

void
Agent::registerSimStream(SimStream* newSimStream) {
    allSimStreams.push_back(newSimStream);
}

bool 
Agent::scheduleEvent(Event* e) {
    // Perform some sanity checks.
    ASSERT(TIME_EQUALS(e->getSentTime(), TIME_INFINITY));
    ASSERT(e->getSenderAgentID() == -1);
    ASSERT(e->isAntiMessage() == false);
    ASSERT(e->getReferenceCount() == 1);
    
    // Fill in the sent time and sender agent id info.
    e->sentTime      = getLVT();
    e->senderAgentID = getAgentID();
    
    // Check to make sure we don't schedule past the simulation end time.
    if (e->getReceiveTime() >= Simulation::getSimulator()->getStopTime()) {
        e->decreaseReference();
        return false;
    }
    
    // Make sure we don't schedule an event with a receive time that is
    // less than or equal to our LVT
    if (e->getReceiveTime() <= getLVT()) {
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
        outputQueue.push_back(e);

        // Keep track of event being scheduled.
        numScheduledEvents++;
        
        return true;
    } else if (Simulation::getSimulator()->scheduleEvent(e)) {
        outputQueue.push_back(e);
       
        // Keep track of event being scheduled.
        numScheduledEvents++;

        // this is to keep track of how many MPI message we use
        if (!Simulation::getSimulator()->isAgentLocal(e->getReceiverAgentID())) {
            numMPIMessages++;
        }
        return true;
    }

    // If control drops here, the event was rejected from
    // scheduling. Clean up.
    e->decreaseReference();
    return false;
}

void
Agent::doRollbackRecovery(const Event* stragglerEvent) {
    //std::cerr << "Rolling back due to: " << *stragglerEvent << std::endl;
    //cout << "Rollback recovery started"<< endl;
    doRestorationPhase(stragglerEvent->getReceiveTime());
    // After state is restored, that means out current time is the restored time
    Time restoredTime = getTime(LVT);
    
    //std::cerr << "*** Agent(" << myID << "): restored time to "
    //          << restoredTime << ", while GVT = " << getTime(GVT)
    //          << std::endl;
   
    doCancellationPhaseInputQueue(restoredTime, stragglerEvent);
    doCancellationPhaseOutputQueue(restoredTime);
    
    //we need to rollback all SimStreams here.
    oss.rollback(restoredTime);
    for (size_t i = 0; (i < allSimStreams.size()); i++) {
        allSimStreams[i]->rollback(restoredTime);
    }

    // Remember to increment the rollback counter
    numRollbacks++;
  
}

void
Agent::doRestorationPhase(const Time& stragglerTime) {
   
    /** OK, here is the plan. First, there is a stragglerTime.Second,
        the stateQueue should be sorted by the nature of its
        containt.We want to find a state with a timestamp smaller than
        the stragglerTime.  Assuming the second reason is correct, if
        we search the stateQueue starting at the back, we not only
        find the state we want quicker, but we also find the time
        that's directly behind the stragglerTime.  Since the
        stateQueue is a double linked list, we can by pass dealing
        with reverse_iterators and keep popping from the back until we
        find the state we want.If we fail to locate state with a
        timestamp that is older than stragglerTime, then we revert
        back to the init_state that we got from the ctor.  The
        following code implements this idea!
    */

    // For debugging
    // cout << "StateQueue B Restoration: ";
    // cout << *this << endl;

    // We set our LVT to INFINITY here in case we don't find a state to
    // restore to -- we can revert to the initial state after the loop.
    setLVT(TIME_INFINITY);
    // Now go and look for a state to restore to.
    ASSERT(!stateQueue.empty());
    while (stateQueue.size() != 1) {
        State* currentState = stateQueue.back();
        if (currentState->getTimeStamp() < stragglerTime) {
            // Destroy the current state
            delete myState;
            // Set the state to the known-good state in the queue
            setState(cloneState(currentState));
            // Set agent's LVT to this state's timestamp
            setLVT(getState()->getTimeStamp());
            // cout << "    State Found for time: "<< LVT << endl;
            break;
        } else {
            // This state refers to a time later than the time of the
            // straggler message. It is no longer valid, so delete it.
            
            // cout << "Deleting CurrentState @ timestamp: "
            //      << currentState->getTimeStamp() << endl;
           
            delete currentState;
            stateQueue.pop_back();
        }
    }
    
    // If LVT is INFINITY, we've have rolled back all the way to beginning.
    if (TIME_EQUALS(getLVT(), TIME_INFINITY)) {
        // There should be only one state in the queue
        ASSERT(stateQueue.size() == 1);
        // cout << "TIME_EQUALS(LVT,INFINITY) = TRUE" << endl;
        delete myState;
        setState(cloneState(stateQueue.front()));
        setLVT(getState()->getTimeStamp());
        // std::cout << "Restored Time To: " << getLVT() << std::endl;
    }
    
    ASSERT(stragglerTime > getLVT());
        
    // For debugging
    // if (stateQueue.size() <= 2) {
    //     cerr << "StateQ After Restore: "
    //          << *this << endl;
    //     << " GVT: " << getTime(GVT)
    //     << " Straggler Time: "<< stragglerTime
    //     << " Restored Time: "<< getTime() << endl;
    // }
}

void
Agent::doCancellationPhaseOutputQueue(const Time& restoredTime) {
  
    AgentIDBoolMap bitMap;
    list<Event*>::iterator outQ_it = outputQueue.begin();
    while (outQ_it != outputQueue.end()) {
        list<Event*>::iterator del_it = outQ_it;
        outQ_it++; 
        Event* currentEvent = (*del_it);
        //check if the event is invalid.
        if (currentEvent->getSentTime() > restoredTime) {
            //check if bitMap to receiver agent has been set.
            if (bitMap[currentEvent->getReceiverAgentID()] == false) {
                bitMap[currentEvent->getReceiverAgentID()] = true;
                if (!(Simulation::getSimulator())->isAgentLocal(currentEvent->getReceiverAgentID())) {
                    currentEvent->makeAntiMessage();
                    //cout << "Making Anti-Message: " << *currentEvent << endl;
                    Simulation::getSimulator()->scheduleEvent(currentEvent);
                } else {
                    // Send an anti-message to the agent on the same
                    // process so that it rolls back. Since pointers
                    // are shared, a duplicate needs to be made.
                    char* flatAntiEvent = new char[sizeof(Event)];
                    memcpy(flatAntiEvent, currentEvent, sizeof(Event));
                    Event* antiEvent = reinterpret_cast<Event*>(flatAntiEvent);
                    antiEvent->setReferenceCount(1);
                    antiEvent->makeAntiMessage();
                    if (!Simulation::getSimulator()->scheduleEvent(antiEvent)) {
                        // The scheduler rejected our anti-message.
                        antiEvent->decreaseReference();
                    }
                }
            }
            // Invalid events automatically get removed
            currentEvent->decreaseReference();
            outputQueue.erase(del_it);
        }
    }
}

void
Agent::doCancellationPhaseInputQueue(const Time& restoredTime,
                                     const Event* straggler) {

    ASSERT(straggler != NULL);

    list<Event*>::iterator inQ_it = inputQueue.begin();
    while (inQ_it != inputQueue.end()) {
        list<Event*>::iterator del_it = inQ_it;
        inQ_it++;
        Event *currentEvent = (*del_it);
        ASSERT(currentEvent->isAntiMessage() == false);

        if (currentEvent->getReceiveTime() <= restoredTime) {
            // This event needs to stay in the input queue - it does
            // not need to be reprocessed or deleted.
            continue;
        }
        
        if ((currentEvent->getSenderAgentID() == myID) &&
            (currentEvent->getSentTime() > restoredTime)) {
            // We sent this event in the future - it gets deleted
            currentEvent->decreaseReference();
            inputQueue.erase(del_it);
            continue;
        }
        
        // If control comes here, the event needs to be rescheduled if
        // it is not from the sender of the straggler AND the
        // straggler is NOT an antimessage
        if (straggler->getSenderAgentID() != currentEvent->getSenderAgentID()) {
            eventPQ->push(currentEvent);
            currentEvent->decreaseReference();
            inputQueue.erase(del_it);
        } else if (currentEvent->getSentTime() >= straggler->getSentTime()) {
            if (straggler->isAntiMessage()) {
                // This is one needs to be nuked.
                currentEvent->decreaseReference();
                inputQueue.erase(del_it);
            } else {
                // Nope, this event is still safe. Even though it's
                // from the sender of the straggler.
                eventPQ->push(currentEvent);
                currentEvent->decreaseReference();
                inputQueue.erase(del_it);
            }
        }
    }
}

void
Agent::cleanStateQueue() {
    while (!stateQueue.empty()) {
        State *currentState = stateQueue.front();
        delete currentState;
        stateQueue.pop_front();
    }
}

void
Agent::cleanInputQueue() {
     while (!inputQueue.empty()) {
        Event *currentEvent = inputQueue.front();
        currentEvent->decreaseReference();
        inputQueue.pop_front();

        // Remember to keep track number of processed events
        numProcessedEvents++;
    }
}

void
Agent::cleanOutputQueue() {
    while (!outputQueue.empty()) {
        Event *currentEvent = outputQueue.front();
        currentEvent->decreaseReference();
        outputQueue.pop_front();
    }
}

void
Agent::garbageCollect(const Time gvt) {

    list<State*>::iterator safe_point_it = stateQueue.begin();
    Time oneBelowGVT = gvt;

    oneBelowGVT = 0;
    while (safe_point_it != stateQueue.end() && (*safe_point_it)->getTimeStamp() < gvt) {
        oneBelowGVT = (*safe_point_it)->getTimeStamp();
        safe_point_it++;
    }
    
    //cerr << "Collecting Garbage now.....oneBelowGVT: " <<
    //oneBelowGVT <<" real GVT: "<<getTime(GVT) << "\n"; cerr <<
    //"States being collected for agent ("<<getAgentID()<<") are:
    //\n";
    
    //now we start looking
    while (stateQueue.front()->getTimeStamp() < oneBelowGVT) {
        State *currentState = stateQueue.front();
        //cerr << "State @ time: " << currentState->getTimeStamp()<<"\n";
        delete currentState;
        stateQueue.pop_front();
    }
    //cerr << *this << endl;

    // The first state should be less than gvt
    ASSERT(stateQueue.front()->getTimeStamp() < gvt);
    
    //second we collect from the inputQueue
    while (!inputQueue.empty() &&
          inputQueue.front()->getReceiveTime() < oneBelowGVT) {
        Event *currentEvent = inputQueue.front();
        currentEvent->decreaseReference();
        inputQueue.pop_front();
        
        //keep track number of processed events
        numCommittedEvents++;
    }
    
    //last we collect from the outputQueue
    while (!outputQueue.empty() &&
          outputQueue.front()->getSentTime() < oneBelowGVT) {
        Event *currentEvent = outputQueue.front();
        currentEvent->decreaseReference();
        outputQueue.pop_front();
    }
    
    
    //we need to garbageCollect all SimStreams here.
    oss.garbageCollect(gvt);
    for (size_t i = 0; (i < allSimStreams.size()); i++) {
        allSimStreams[i]->garbageCollect(gvt);
    }
}

Time
Agent::getTime(TimeType timeType) const {
    switch(timeType) {
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
statePrinter(ostream& os, list<muse::State*> state_q ) {
    //list<muse::State*>::reverse_iterator rit = state_q.rbegin();
    //for (; rit != state_q.rend(); rit++) {
    list<muse::State*>::iterator it = state_q.begin();
    for (; it != state_q.end(); it++) {
        os << (*it)->getTimeStamp() << ",";
    }
    os << ")]    " ;
    return os;
}

ostream&
operator<<(ostream& os, const muse::Agent& agent) {
    if (!agent.eventPQ->empty()) {
        os << "Agent[id="           << agent.getAgentID()    << ","
           << "# of Events in heap="<< agent.eventPQ->size() << ","
           << "top "                << *agent.eventPQ->top() << ","
           << "StateQueue: ("       << statePrinter(os,agent.stateQueue); //<< ")]" ;
    } else {
        os << "Agent[id="           << agent.getAgentID()    << ","
           << "top EVENT:"          << "EMPTY"               << ","
           << "StateQueue: ("       << statePrinter(os,agent.stateQueue); //<< ")]" ;
    }
    return os;
}



#endif
 
