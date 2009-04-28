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
#include <cstdlib>
#include "Simulation.h"
#include "BinaryHeapWrapper.h"

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
Scheduler::updateKey(void * pointer, Time old_top_time){
    AgentPQ::pointer ptr = reinterpret_cast<AgentPQ::pointer>(pointer);
    agent_pq.update(ptr, old_top_time);
}

bool
Scheduler::processNextAgentEvents(){
    if (agent_pq.top()->eventPQ->empty()){
        return false;
    }
    
    Agent * agent = agent_pq.top();
    //lets get the old_top_time
    Time old_top_time = agent->getTopTime();
    bool result = agent->processNextEvents();
    updateKey(reinterpret_cast<AgentPQ::pointer>(agent->fibHeapPtr),old_top_time);
    return result;
}//end processNextAgentEvents

bool
Scheduler::scheduleEvent( Event *e){
    //make sure the recevier agent has an entry
    AgentID agent_id = e->getReceiverAgentID();
    AgentIDAgentPointerMap::iterator entry = agentMap.find(agent_id);
    Agent* agent = (entry != agentMap.end()) ? entry->second : NULL ;
    
    if (agent == NULL) {
        cerr << "Trying to schedule ("<<*e<<") to unknown agent\n";
        cerr << "Available agents are: \n";
        AgentIDAgentPointerMap::iterator it = agentMap.begin();
        for (;it!=agentMap.end(); it++) {
            cerr << *(it->second) << "\n";
        }
        cerr << "Trying to schedule to local agent that doesn't exist" <<endl;
        abort();
        //THIS CASE SHOULD NEVER HAPPEN
    }

    //will use this to figure out if we need to change our key in
    //scheduler
    Time old_top_time = agent->getTopTime();

    //now check if this is a rollback!
    if ( !checkAndHandleRollback(e, agent) && e->isAntiMessage() ){
        handleFutureAntiMessage(e, agent);
        //updateKey(agent->fibHeapPtr,old_top_time);
        return false;
    }

    //this handles cases where the anti-message was for the past
    if ( e->isAntiMessage() ){
        //this means that it was already rollback'd so we just need to return false
        updateKey(agent->fibHeapPtr,old_top_time);
        return false;
    }
    
    ASSERT(e->isAntiMessage() == false );
    
   
    //push to agent's heap
    agent->eventPQ->push(e);
    std::cerr << "Scheduled: " << *e << std::endl;
    
    updateKey(agent->fibHeapPtr,old_top_time);
    
    //everything went well!
    return true;
}//end scheduleEvent

bool
Scheduler::checkAndHandleRollback(const Event * e,  Agent * agent){
    if ( e->getReceiveTime() <= agent->getLVT() ){
        ASSERT(e->getSenderAgentID() !=  e->getReceiverAgentID());
        //std::cout << "Rollingback due to: " << *e << std::endl;
        agent->doRollbackRecovery(e);
        if ( e->getReceiveTime() <= agent->getLVT() ) { //CHECK WITH RAO ABOU THIS CHANGE from <= TO <
            // Error condition.
            std::cerr << "Rollback logic did not restore state correctly?\n";
            std::cerr << "Agent info:\n" << *agent << std::endl;
            abort();
        }//end if
        return true;
    }//end if
    return false;
}

void
Scheduler::handleFutureAntiMessage(const Event * e,Agent * agent){
    
    //std::cout << "*Cancelling due to: " << *e << std::endl;
    // This event is an anti-message we must remove it and
    // future events from this agent.
    bool foundAtleastOne = agent->eventPQ->removeFutureEvents(e);
     
    // We must have deleted at least one event for this anit-message
    if (false && !foundAtleastOne ) { //1. MADE A CHANGE TO MAKE SURE NOT EMPTY
        cerr << "eventPQ size: " << agent->eventPQ->size() << endl;
        std::cerr << "Did not find an event to cancel for anti-message \n"
                  << *e << std::endl;
        // Print the fibonacci heap for reference purposes
        std::cerr << "The list of pending events (at LVT="
                  << agent->getLVT() << "):\n";
       
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
}

Scheduler::~Scheduler(){}//end Scheduler


Time
Scheduler::getNextEventTime()  {
    if (agent_pq.empty()) {
        // The queue is empty. So return infinity.
        return TIME_INFINITY;
    }
    // Obtain reference to the top agent in the priority queue.
    //getTopTime returns the time or TIME_INFINITY if agent's eventPQ is empty
    return  agent_pq.top()->getTopTime();
   
}

#endif
