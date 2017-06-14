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
//
//---------------------------------------------------------------------------

#include "Agent.h"
#include "Simulation.h"
#include "HashMap.h"
#include "BinaryHeapWrapper.h"
#include <iostream>
#include <cstdlib>
#include "EventQueue.h"

using namespace muse;

Agent::Agent(AgentID id, State* agentState)
    : myID(id), lvt(0), myState(agentState), mustSaveState(true),
      numRollbacks(0), numScheduledEvents(0), numProcessedEvents(0),
      numMPIMessages(0), numCommittedEvents(0), numSchedules(0) {
    // The event queue creation now happens in respective queue
    // classes derived from EventQueue (in addAgent() method).

    // Initialize fibonacci heap cross references
    fibHeapPtr = NULL;
    oldTopTime = TIME_INFINITY;
}

Agent::~Agent() {
    // Let's make sure we dont have any left over events because
    // finalize() method should have properly cleaned up the Priority
    // Queue
    ASSERT(inputQueue.empty());
    ASSERT(outputQueue.empty());
    ASSERT(stateQueue.empty());
    delete myState;
}

void
Agent::saveState() {
    if (mustSaveState || stateQueue.empty()) {
        // We need at least one entry in the state queue to streamline
        // operations.  Clone the current state so it can be saved
        State* state = cloneState(getState());
        state->timestamp = getLVT();
        
        // The states should be monotomically increasing in timestamp
        ASSERT(stateQueue.empty() ||
               (stateQueue.back()->getTimeStamp() < state->getTimeStamp()));
        // Save current state
        stateQueue.push_back(state);
    } else if (!mustSaveState) {
        ASSERT(stateQueue.size() == 1);
        stateQueue.front()->timestamp = getLVT();
    }
    // Save the states of all of the SimStreams.  This is needed to
    // handle optimistic I/O correctly, immaterial of whether state
    // saving is enabled/disabled.
    oss.saveState(getLVT());
    for (size_t i = 0; (i < allSimStreams.size()); i++) {
        allSimStreams[i]->saveState(getLVT());
    }
    DEBUG(std::cout << "Agent " << getAgentID() << " saved state at "
                    << getLVT() << std::endl);
}

void
Agent::processNextEvents(muse::EventContainer& events) {
    // The events cannot be empty
    ASSERT(!events.empty());
    // Add the events to our input queue only when state saving is
    // enabled -- that is we have more than 1 process and rollbacks
    // are possible.
    if (mustSaveState) {
        for (EventContainer::iterator curr = events.begin();
             (curr != events.end()); curr++) {
            // Avoiding the following 2 redundant operations:
            // (*curr)->increaseReference();  // Add to inputQueue
            // (*curr)->decreaseReference();  // remove from events container
            inputQueue.push_back(*curr);
        }
    } else {
        // keep track number of processed events -- this would be
        // normally done during garbage collection.  However, in 1 LP
        // mode we do not add events to input queue and consequently
        // we need to track it here.
        numCommittedEvents += events.size();
    }
    DEBUG(std::cout << "Agent " << getAgentID() << " is scheduled to process "
                    << "events at time: " << events.front()->getReceiveTime());
    ASSERT(events.front()->getReceiveTime() > getState()->timestamp);
    // Set the LVT and timestamp
    setLVT(events.front()->getReceiveTime());
    getState()->timestamp = getLVT();
    // Let the derived class actually process events that it needs to.
    executeTask(&events);
    
    // Increment the numProcessedEvents and numScheduled counters
    numProcessedEvents += events.size();
    numSchedules++;
    
    // Save the state (if needed) now that events have been processed.
    saveState();

    if (!mustSaveState) {
        // Decrease reference and free-up events as we are not adding to 
        // input queue for handling rollbacks that cannot occur in this case.
        for (EventContainer::iterator curr = events.begin();
             (curr != events.end()); curr++) {
            ASSERT( (*curr)->getReferenceCount() == 1 );
            (*curr)->decreaseReference();
        }
    }
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
    ASSERT(e->getSenderAgentID()  == -1);
    ASSERT(e->isAntiMessage()     == false);
    ASSERT(e->getReferenceCount() == 1);
    
    // Fill in the sent time and sender agent id info.
    e->sentTime      = getLVT();
    e->senderAgentID = getAgentID();
    ASSERT(e->sentTime >= getTime(GVT));
    
    // Check to make sure we don't schedule past the simulation end time.
    if (e->getReceiveTime() >= Simulation::getSimulator()->getStopTime()) {
        e->decreaseReference();
        return false;
    }
    
    // Make sure we don't schedule an event with a receive time that is
    // less than or equal to our LVT
    if (e->getReceiveTime() <= getLVT()) {
        std::cerr << "Attempt to schedule an event with a smaller or equal "
                  << "timestamp as LVT, this is impossible as it violates"
                  << "causality constraints." << std::endl;
        e->decreaseReference();
        abort();
    }
    if (Simulation::getSimulator()->scheduleEvent(e)) {
        DEBUG(std::cout << "Scheduled: " << *e << std::endl);
        // Add event to our output queue only when more than 1 process
        // is used for parallel simulation
        if (mustSaveState) {
            outputQueue.push_back(e);
        } else {
            // We don't add this event to output queue so decrease reference
            ASSERT( e->getReferenceCount() > 1 );
            e->decreaseReference();
        }
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
Agent::doRollbackRecovery(const Event* stragglerEvent,
                          muse::EventQueue& reschedule) {
    DEBUG(std::cout << "Rolling back due to: " << *stragglerEvent << std::endl);
    doRestorationPhase(stragglerEvent->getReceiveTime());
    // After state is restored, that means out current time is the restored time
    Time restoredTime = getTime(LVT);
    
    DEBUG(std::cout << "*** Agent(" << myID << "): restored time to "
                    << restoredTime << ", while GVT = " << getTime(GVT)
                    << std::endl);
    
    // First reschedule events.  This must happen before output queue
    // processing to handle cyclic rollback chains of the form: A1 ->
    // A2 -> A3 -> A1.  The Scheduler::handleFutureAntiMessages() also
    // counts on input queue clean-up to happen first.
    muse::EventContainer eventsToReschedule;
    doCancellationPhaseInputQueue(restoredTime, stragglerEvent,
                                  eventsToReschedule);
    reschedule.enqueue(this, eventsToReschedule);
    
    // Next clean-up output queue. This will cause future clean-up of
    // input queue if there are cyclic dependencies.
    doCancellationPhaseOutputQueue(restoredTime);
    
    // We need to rollback all SimStreams here.
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
    ASSERT( stragglerTime >= getTime(GVT) );
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
            stateQueue.pop_back();
            delete currentState;
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
    AgentIDBoolMap bitMap;  // track if anti-msg has been sent to an agent
    List<Event*>::iterator outQ_it = outputQueue.begin();
    while (outQ_it != outputQueue.end()) {
        Event* const currentEvent = *outQ_it;
        // check if the event is invalid.
        if (currentEvent->getSentTime() > restoredTime) {
            const AgentID receiver = currentEvent->getReceiverAgentID();
            // check if bitMap to receiver agent has been set.
            if (bitMap[receiver] == false) {
                bitMap[receiver] = true;  // send anit-message flag.
                if (!Simulation::getSimulator()->isAgentLocal(receiver)) {
                    currentEvent->makeAntiMessage();
                    // cout << "Making Anti-Message: " << *currentEvent << endl;
                    Simulation::getSimulator()->scheduleEvent(currentEvent);
                } else {
                    // Send an anti-message to the agent on the same
                    // process so that it rolls back. Since pointers
                    // are shared, a duplicate needs to be made.
                    Event* const antiEvent =
                        Event::create(receiver, currentEvent->getReceiveTime());
                    // Setup sender and send-time information.
                    antiEvent->senderAgentID = currentEvent->senderAgentID;
                    antiEvent->sentTime      = currentEvent->sentTime;
                    antiEvent->makeAntiMessage();
                    if (!Simulation::getSimulator()->scheduleEvent(antiEvent)) {
                        // The scheduler rejected our anti-message.
                        antiEvent->decreaseReference();
                    }
                }
            }
            // Invalid events automatically get removed
            currentEvent->decreaseReference();
            outQ_it = outputQueue.erase(outQ_it);  // erase & update iterator
        } else {
            outQ_it++;  // Onto the next event to be checked
        }
    }
}

void
Agent::doCancellationPhaseInputQueue(const Time& restoredTime,
                                     const Event* straggler,
                                     muse::EventContainer& reschedule) {
    ASSERT(straggler != NULL);
    List<Event*>::iterator del_it = inputQueue.begin();
    while (del_it != inputQueue.end()) {
        Event *currentEvent = (*del_it);
        ASSERT(currentEvent != NULL);
        ASSERT(currentEvent->isAntiMessage() == false);
        // There are different cases to be applied for processing below.
        if (currentEvent->getReceiveTime() <= restoredTime) {
            // This event needs to stay in the input queue - it does
            // not need to be reprocessed or deleted.
            del_it++;   // onto the next entry
            continue;
        }
        
        if ((currentEvent->getSenderAgentID() == myID) &&
            (currentEvent->getSentTime() > restoredTime)) {
            // We sent this event to ourselves in the future - it gets deleted
            del_it = inputQueue.erase(del_it);  // remove & update iterator
            currentEvent->decreaseReference();  // release reference
            continue;
        }
        
        // If control comes here, the event needs to be rescheduled if
        // it is not from the sender of the straggler AND the
        // straggler is NOT an antimessage.  The following
        // if-statement is a NOT of the one case to be ignored.  It is
        // left as a NOT in the hopes that the code is more readable.
        // This code was modified on Jul 16 2014 by Rao as a part of
        // addressing a missing/unhandled-case.  The code is a
        // simplification of a series of if-else statements that were
        // here prior to this revision.
        if (!((straggler->getSenderAgentID() == currentEvent->getSenderAgentID()) &&
              (currentEvent->getSentTime() >= straggler->getSentTime()) &&
              (straggler->isAntiMessage()))) {
            reschedule.push_back(currentEvent);
            DEBUG(std::cout << "*Rescheduling: " << *currentEvent << std::endl);
        } else {
            // Current event is discarded as it is not rescheduled.
            currentEvent->decreaseReference();
        }
        del_it = inputQueue.erase(del_it);  // remove & update iterator
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
        // Remember to keep track number of committed events
        numCommittedEvents++;
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
    // First find a state in state queue that is below GVT so we will
    // always have a state to rollback to.
    List<State*>::iterator safe_point_it = stateQueue.begin();
    Time oneBelowGVT = 0;
    ASSERT(!stateQueue.empty());    
    while ((safe_point_it != stateQueue.end()) &&
           ((*safe_point_it)->getTimeStamp() < gvt)) {
        oneBelowGVT = (*safe_point_it)->getTimeStamp();
        safe_point_it++;
    }
    
    DEBUG(std::cout << "Garbage collecting for agent " << getAgentID()
                    << ", oneBelowGVT: " << oneBelowGVT << ", real GVT: "
                    << getTime(GVT) << std::endl);
    
    // now we start looking
    while (stateQueue.front()->getTimeStamp() < oneBelowGVT) {
        State *currentState = stateQueue.front();
        //cerr << "State @ time: " << currentState->getTimeStamp()<<"\n";
        stateQueue.pop_front();        
        delete currentState;
    }
    //cerr << *this << endl;

    // The first state should be less than gvt
    ASSERT(!stateQueue.empty());
    ASSERT(!mustSaveState || (stateQueue.front()->getTimeStamp() < gvt));
    
    //second we collect from the inputQueue
    while (!inputQueue.empty() &&
           (inputQueue.front()->getReceiveTime() < oneBelowGVT)) {
        Event *currentEvent = inputQueue.front();
        currentEvent->decreaseReference();
        inputQueue.pop_front();
        //keep track number of processed events
        numCommittedEvents++;
    }
    
    //last we collect from the outputQueue
    while (!outputQueue.empty() &&
           (outputQueue.front()->getSentTime() < oneBelowGVT)) {
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

bool
Agent::agentComp::operator()(const Agent *lhs, const Agent *rhs) const {
    Time lhs_time = (lhs->schedRef.eventPQ->empty() ? TIME_INFINITY :
		     lhs->schedRef.eventPQ->top()->getReceiveTime());
    Time rhs_time = (rhs->schedRef.eventPQ->empty() ? TIME_INFINITY :
		     rhs->schedRef.eventPQ->top()->getReceiveTime());
    return (lhs_time >= rhs_time);
}


ostream&
statePrinter(ostream& os, List<muse::State*> state_q ) {
    List<muse::State*>::iterator it = state_q.begin();
    os << "(";
    for (; it != state_q.end(); it++) {
        os << (*it)->getTimeStamp() << ",";
    }
    os << ")]" ;
    return os;
}

ostream&
operator<<(ostream& os, const muse::Agent& agent) {
    os << "Agent[id: "       << agent.getAgentID() << "; "
       << "Events in heap: " <<  agent.schedRef.eventPQ->size();
    if (!agent.schedRef.eventPQ->empty()) {
	DEBUG(os << "; Top Event: " << *agent.schedRef.eventPQ->top());
    } else {
	DEBUG(os << "; Top Event: none");
    }
    DEBUG(os << "; StateQueue: " << statePrinter(os,agent.stateQueue));
    os << "]";
    return os; 
}

void
Agent::dumpStats(std::ostream& os, const bool printHeader) const {
    if (printHeader) {
        os << "AgentID\tLVT\tInputQueueSize\tOutputQueueSize\tStateQueueSize\t"
           << "EventQueueSize\t#Sched.Evts\t#Processed.Evts\t#Rollbacks\t"
           << "#MpiMsgs\n";
    }
    // Ensure that the order of output matches the header line above.
    const std::string TAB = "\t";
    os << getAgentID()       << TAB
       << lvt                << TAB
       << inputQueue.size()  << TAB
       << outputQueue.size() << TAB
       << stateQueue.size()  << TAB
       << schedRef.eventPQ->size() << TAB
       << numScheduledEvents << TAB
       << numProcessedEvents << TAB
       << numRollbacks       << TAB
       << numMPIMessages     << std::endl;
}

#endif
