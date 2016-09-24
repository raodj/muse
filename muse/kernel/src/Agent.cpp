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
#include "ThreeTierHeapEventQueue.h"
#include <iostream>
#include <cstdlib>
#include "EventQueue.h"

using namespace muse;

Agent::Agent(AgentID id, State* agentState)
    : myID(id), lvt(0), myState(agentState), mustSaveState(true),
      numRollbacks(0), numScheduledEvents(0), numProcessedEvents(0),
      numMPIMessages(0), numCommittedEvents(0), numSchedules(0) {
    // Make an Event Priority Queue
    eventPQ = new BinaryHeapWrapper();
    eventPQ = new BinaryHeap<muse::Tier2Entry, muse::EventComp>();
    // Initialize fibonacci heap cross references
    fibHeapPtr = NULL;
    oldTopTime = TIME_INFINITY;
}

Agent::~Agent() {
    // Let's make sure we dont have any left over events because
    // finalize() method should have properly cleaned up the Priority
    // Queue
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *bh;
    bh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (eventPQ); 
    //BinaryHeapWrapper *bhw = reinterpret_cast<BinaryHeapWrapper*>(eventPQ); 
    ASSERT(bh->empty());
    //ASSERT(bhw->empty());
    ASSERT(inputQueue.empty());
    ASSERT(outputQueue.empty());
    ASSERT(stateQueue.empty());
    
    delete bh;
    //delete bhw;
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
                    << getLVT() << ". Next event time: "
                    << (!eventPQ->empty() ? eventPQ->top()->getReceiveTime() :
                        TIME_INFINITY) << std::endl);
}

void
Agent::processNextEvents(muse::EventContainer& events) {
    // The events cannot be empty
    ASSERT(!events.empty());
    // Add the events to our input queue
    for (EventContainer::iterator curr = events.begin(); (curr != events.end());
         curr++) {
        // Avoiding the following 2 redundant operations:
        // (*curr)->increaseReference();  // Add to inputQueue
        // (*curr)->decreaseReference();  // remove from events container
        inputQueue.push_back(*curr);
    }
    ASSERT(events.front()->getReceiveTime() > getState()->timestamp);
    // Set the LVT and timestamp
    setLVT(events.front()->getReceiveTime());
    getState()->timestamp = getLVT();
    executeTask(&events);
    
    // Increment the numProcessedEvents and numScheduled counters
    numProcessedEvents += events.size();
    numSchedules++;
    
    // Save the state (if needed) now that events have been processed.
    saveState();
}

void
Agent::getNextEvents(EventContainer& container) {
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *bh;
    bh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (eventPQ); 
    ASSERT(container.empty());
    ASSERT( (bh->top().getEvent() != NULL) );
    
    const Time currTime = bh->top().getEvent()->getReceiveTime();  
    do {
        Event* event = bh->top().getEvent();  
        // We should never process an anti-message.
        if (event->isAntiMessage()) { 
            std::cerr << "Anti-message Processing: " << *event << std::endl;
            std::cerr << "Trying to process an anti-message event, "
                      << "please notify MUSE developers of this issue" << std::endl;
            abort();
        }
    
        // Ensure that the top event is greater than LVT
        if (event->getReceiveTime() <= getLVT()) {
            std::cerr << "Agent is being scheduled to process an event ("
                      << *event << ") that is at or below it LVT (LVT="
                      << getLVT() << ", GVT=" << getTime(GVT)
                      << "). This is a serious error. Aborting.\n";
            std::cerr << *this << std::endl;
            abort();
        }

        ASSERT(event->getReferenceCount() < 3);
        
        // We add the top event we popped to the event container
        event->increaseReference(); 
        container.push_back(event);

        DEBUG(std::cout << "Delivering: " << *event << std::endl);
        
        // inputQueue.push_back(event);
        
        // Finally it is safe to remove this event from the eventPQ as
        // it has been added to the inputQueue
        bh->pop();
    } while ((!bh->empty()) &&
             (TIME_EQUALS(bh->top().getEvent()->getReceiveTime(), currTime)));   
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
        std::cerr << "Attemp to schedule an event with a smaller or equal "
                  << "timestamp as LVT, this is impossible as it violates"
                  << "causality constraints." << std::endl;
        e->decreaseReference();
        abort();
    }
    if (Simulation::getSimulator()->scheduleEvent(e)) {
        DEBUG(std::cout << "Scheduled: " << *e << std::endl);
        // Add event to our output queue
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
                                     const Event* straggler,
                                     muse::EventContainer& reschedule) {
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
            // We sent this event to ourselves in the future - it gets deleted
            currentEvent->decreaseReference();
            inputQueue.erase(del_it);
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
        inputQueue.erase(del_it);
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
    // First find a state in state queue that is below GVT so we will
    // always have a state to rollback to.
    list<State*>::iterator safe_point_it = stateQueue.begin();
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

Time
Agent::getTopTime() const {
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *bh;
    bh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (eventPQ); 
    return bh->empty() ? TIME_INFINITY : bh->top().getEvent()->getReceiveTime();
}

bool
Agent::agentComp::operator()(const Agent *lhs, const Agent *rhs) const {
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *lbh;
    lbh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (lhs->eventPQ);
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *rbh;
    rbh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (rhs->eventPQ);
    Time lhs_time = (lbh->empty() ? TIME_INFINITY :
		     lbh->top().getEvent()->getReceiveTime());
    Time rhs_time = (rbh->empty() ? TIME_INFINITY :
		     rbh->top().getEvent()->getReceiveTime());
    return (lhs_time >= rhs_time);
}


ostream&
statePrinter(ostream& os, list<muse::State*> state_q ) {
    list<muse::State*>::iterator it = state_q.begin();
    os << "(";
    for (; it != state_q.end(); it++) {
        os << (*it)->getTimeStamp() << ",";
    }
    os << ")]" ;
    return os;
}

ostream&
operator<<(ostream& os, const muse::Agent& agent) {
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *bh;
    bh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (agent.eventPQ); 
    os << "Agent[id: "       << agent.getAgentID() << "; "
       << "Events in heap: " << bh->size();
    if (!bh->empty()) {
	DEBUG(os << "; Top Event: " << *agent.bh->top());
    } else {
	DEBUG(os << "; Top Event: none");
    }
    DEBUG(os << "; StateQueue: " << statePrinter(os,agent.stateQueue));
    os << "]";
    return os;
}

void
Agent::dumpStats(std::ostream& os, const bool printHeader) const {
    BinaryHeap<muse::Tier2Entry, muse::EventComp> *bh;
    bh = reinterpret_cast<BinaryHeap<muse::Tier2Entry, muse::EventComp>*>
            (eventPQ); 
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
       << bh->size()         << TAB
       << numScheduledEvents << TAB
       << numProcessedEvents << TAB
       << numRollbacks       << TAB
       << numMPIMessages     << std::endl;
}

#endif
