#ifndef _MUSE_SCHEDULER_CPP_
#define _MUSE_SCHEDULER_CPP_

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
//
//
//---------------------------------------------------------------------------

#include "Scheduler.h"
#include "f_heap.h"
#include <cstdlib>
#include "Simulation.h"

using namespace muse;
Scheduler::Scheduler(){}

bool
Scheduler::addAgentToScheduler(Agent * agent){
    if (agentMap[agent->getAgentID()] == NULL) {
        agentMap[agent->getAgentID()] = agent;
        agent->fibHeapPtr = reinterpret_cast<void *>(agent_pq.push(agent));
        return true;
    }
    return false;
}//end addAgentToScheduler

void
Scheduler::changeKey(void * pointer, Agent * agent){
    AgentPQ::pointer ptr = reinterpret_cast<AgentPQ::pointer>(pointer);
    agent_pq.increase(ptr, agent);
}

bool
Scheduler::processNextAgentEvents(){
    if (agent_pq.top()->eventPQ->empty()){
        return false;
    }
    
    Agent * agent = agent_pq.top();
    bool result = agent->processNextEvents();
    agent_pq.decrease(reinterpret_cast<AgentPQ::pointer>( agent->fibHeapPtr),agent);
    return result;
}//end processNextAgentEvents

bool
Scheduler::scheduleEvent( Event *e){
    //make sure the recevier agent has an entry
    AgentID agent_id = e->getReceiverAgentID();
    AgentIDAgentPointerMap::iterator entry = agentMap.find(agent_id);
    Agent* agent = (entry != agentMap.end()) ? entry->second : NULL ;
    
    if (agent == NULL) {
        cerr << "Trying to schedule to local agent that doesn't exist" <<endl;
        abort();
        //THIS CASE SHOULD NEVER HAPPEN
    }

    //now check if this is a rollback!
    checkAndHandleRollback(e, agent);

    //check if this is an anti-message!
    if (checkAndHandleAntiMessage(e, agent)) return false;


    ASSERT(e->isAntiMessage() == false );
    
    //will use this to figure out if we need to change our key in
    //scheduler
    Time old_receive_time = (!agent->eventPQ->empty()) ? agent->eventPQ->top()->getReceiveTime() : TIME_INFINITY;

    //increase reference and push to agent's heap
    e->increaseReference();
    agent->eventPQ->push(e);
    std::cout << "Scheduled: " << *e << std::endl;
    
    //we have to change if the event receive time has a smaller key
    //then the top event in the heap.
    if ( e->getReceiveTime() < old_receive_time  ) {
        changeKey(agent->fibHeapPtr,agent);
    }

    //everything went well!
    return true;
}//end scheduleEvent


bool
Scheduler::checkAndHandleRollback(const Event * e,  Agent * agent){
    if ( e->getReceiveTime() <= agent->getLVT() ){
        ASSERT(e->getSenderAgentID() !=  e->getReceiverAgentID());
        std::cout << "Rollingback due to: " << *e << std::endl;
        agent->doRollbackRecovery(e);
        if (e->getReceiveTime() <= agent->getLVT()) {
            // Error condition.
            std::cerr << "Rollback logic did not restore state correctly?\n";
            std::cerr << "Agent info:\n" << *agent << std::endl;
            abort();
        }//end if
    }//end if
}

bool
Scheduler::checkAndHandleAntiMessage(const Event * e,Agent * agent){
    
    if ( e->isAntiMessage() ) {
        if( e->getReceiveTime() >= agent->getLVT() ) {
            std::cout << "*Cancelling due to: " << *e << std::endl;
            // This event is an anti-message we must remove it and
            // future events from this agent.
            bool foundAtleastOne = false;
            Agent::EventPQ::iterator it = agent->eventPQ->begin();
            while (it != agent->eventPQ->end()) {
                Agent::EventPQ::iterator del_it = it;
                it++;
                // first check if the event matchs the straggler
                // event's senderAgentID
                if ((*del_it)->getSenderAgentID() == e->getSenderAgentID() &&
                    ((*del_it)->getReceiveTime() >= e->getReceiveTime())) {
                    //if the receive times are the same or greater then we
                    //dont need this event
                    //cerr<< "***Deleting Event: " << *(*del_it) << endl;
                    //cout << "---found useless future event  deleting from fib heap"<<endl;
                    std::cout << "Future Cancelling: " << *(*del_it) << std::endl;
                    agent->eventPQ->remove(del_it.getNode());
                    foundAtleastOne = true;
                }// end if
            }//end while
            
            // We must have deleted at least one event for this anit-message
            if (!foundAtleastOne) {
                cerr << "eventPQ size: " << agent->eventPQ->size() << endl;
                std::cerr << "Did not find an event to cancel for anti-message \n"
                          << *e << std::endl;
                // Print the fibonacci heap for reference purposes
                std::cerr << "The list of pending events (at LVT="
                          << agent->getLVT() << "):\n";
                agent->eventPQ->prettyPrint(std::cerr);
                std::cerr << ". This is a serious error. Aborting."
                          << std::endl;
                
                list<Event*>::iterator it = agent->inputQueue.begin();
                cerr << "InputQueue looks " <<endl;
                while ( it != agent->inputQueue.end()) {
                    cerr << *(*it) <<endl;
                    it++;
                }
                abort();
            }//end if 
            //if ( Simulation::getSimulator()->isAgentLocal(e->getSenderAgentID()) == false )
        }
        //ok it was an anti-message!
        return true;
    }

    //means this event was not an anti-message
    return false;
}

Scheduler::~Scheduler(){}//end Scheduler


Time
Scheduler::getNextEventTime() const {
    if (agent_pq.empty()) {
        // The queue is empty. So return infinity.
        return TIME_INFINITY;
    }
    // Obtain reference to the top agent in the priority queue.
    const Agent *agent = agent_pq.top();
    //cout << "TOP agent id is: " << agent->getAgentID() << endl;
    //cout << "Agent address: " << 
    // Now, look at the agent's sub-queue to determine top event.
    if (agent->eventPQ->empty()) {
        // No events on the top-most queue.
        //cout << "Agent eventPQ TOp is Empty" <<endl;
        return TIME_INFINITY;
        
    }
    //cout << "TOP @ scheduler is: " << *agent->eventPQ->top() << endl;
    // Use the top-agent's top-event.
    return agent->eventPQ->top()->getReceiveTime();
}

#endif
