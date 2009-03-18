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
    //cout << "In changeKey: changing key for agent " << ptr->data()->getAgentID() << endl;
    agent_pq.increase(ptr, agent);
}

bool
Scheduler::processNextAgentEvents(){
    if (agent_pq.top()->eventPQ->empty()){
        //changeKey(agent_pq.top()->fibHeapPtr,agent_pq.top()  );
        //cout << "Agent eventPQ top is empty changing key" <<endl;
        return false;
    }

    //for debugging
    //cout << "AgentPQ is: " <<endl;
    //AgentPQ::iterator it = agent_pq.begin();
    //for(;it != agent_pq.end();it++ ){
    //    cout << "Agent: " << (*it)->getAgentID() 
    //         << " has EventPQ size: " << (*it)->eventPQ->size() <<endl;
    // }

    
    Agent * agent = agent_pq.top();
    cout << "\n\nTop before is Agent: " << agent_pq.top()->getAgentID() << endl;
    //cout << "Agent eventPQ top empty ? " << ((agent->eventPQ->empty()) ? "true" : "false") << endl;
    bool result = agent->processNextEvents();
    agent_pq.decrease(reinterpret_cast<AgentPQ::pointer>( agent->fibHeapPtr),agent);
    cout << "Top after is Agent: " << agent_pq.top()->getAgentID() << endl <<endl;
    return result;
}//end processNextAgentEvents

bool
Scheduler::scheduleEvent( Event *e){
  
    //make sure the recevier agent has an entry
    Agent * agent = agentMap[e->getReceiverAgentID()];
    if ( agent == NULL) return false;

    //first check if this is a rollback!
    if (e->getReceiveTime() <= agent->getLVT()){
        ASSERT(e->getSenderAgentID() !=  e->getReceiverAgentID());
        agent->doRollbackRecovery(e);
    }
    
    //check if event is an anti-message
    if (e->isAntiMessage() ){
        //since it is an anti-message we don't need to reprocess the event.
        
        //check if this anti-message is for the furture time.
        if (e->getReceiveTime() > agent->getLVT()) {
            //we must remove it and its subtree of events.
            //cout << "-Detected Anti-message for the future to agent: "<< e->getReceiverAgentID()<<endl;
            Agent::EventPQ::iterator it = agent->eventPQ->begin();
            
            while ( it != agent->eventPQ->end() ) {
                Agent::EventPQ::iterator del_it = it;
                it++;
                
                //first check if the event matchs the straggler event's senderAgentID
                if ((*del_it)->getSenderAgentID() == e->getSenderAgentID()){
                    
                    //if the receive times are the same or greater then we dont need this event
                    if ( (*del_it)->getReceiveTime() >= e->getReceiveTime() ){
                        //cout << "---found useless future event  deleting from fib heap"<<endl;
                        agent->eventPQ->remove(del_it.getNode());
                    }
                }
            }//end while
        }
        return false;   
    }//end anti-message check if

    //will use this to figure out if we need to change our key in
    //scheduler
    Time old_receive_time = TIME_INFINITY;
    if (!agent->eventPQ->empty()){
        old_receive_time = agent->eventPQ->top()->getReceiveTime();
    }
    e->increaseReference();
    agent->eventPQ->push(e);
    cout << "Pushed Event From Agent: "<<e->getSenderAgentID() << " to Agent: "
         << agent->getAgentID()<< " for time: " << e->getReceiveTime() << endl;
    
    //now lets make sure that the heap is still valid
    //we have to change if the event receive time has a smaller key
    //then the top event in the heap.
    if ( e->getReceiveTime() < old_receive_time  ) {
        //we need to call for the key change
        //cout <<"**** Agent: "<<agent->getAgentID() << "****changed key in Scheduler::scheduleEvent" <<endl;
        changeKey(agent->fibHeapPtr,agent);
    }

    //for debugging
    //cout << "\nTop Agent in AgentPQ is: " <<  agent_pq.top()->getAgentID();
    //cout << " Currently AgentPQ is: " <<endl;
    // AgentPQ::iterator it = agent_pq.begin();
    //for(;it != agent_pq.end();it++ ){
    //    cout << *(*it) <<endl; 
    //}
    agent_pq.prettyPrint(std::cout);
    return true;
}//end scheduleEvent

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
