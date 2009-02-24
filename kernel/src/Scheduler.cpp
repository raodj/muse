

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
        agent->fibHeapPtr = (void *) agent_pq.push(agent);
        return true;
    }
    return false;
}//end addAgentToScheduler


bool
Scheduler::processNextAgentEvents(){
    Agent * agent = agent_pq.top();
    bool result = agent->processNextEvents();
    
    agent_pq.change((AgentPQ::pointer) agent->fibHeapPtr, agent);
    
    return result;
}//end processNextAgentEvents

bool
Scheduler::scheduleEvent( Event *e){
  
    //make sure the recevier agent has an entry
    Agent * agent = agentMap[e->getReceiverAgentID()];
    if ( agent == NULL) return false;

    //first check if this is a rollback!
    if (e->getReceiveTime() <= agent->LVT){
        ASSERT(e->getSenderAgentID() !=  e->getReceiverAgentID());
      
       
        //cout << "Detected a ROLLBACK @ agent: "<<agent->getAgentID() << endl;
        //cout <<"straggler event: " << *e <<endl;
        //cout << "Straggler Time: "<< e->getReceiveTime() <<endl;
        //cout << "Current LVT: "<< agent->getLVT() <<endl <<endl;
        //cout << "myState timestamp: "<< 
	//debug info print out
        
	//cout << "[Scheduler] - Output Queue Size: "<< agent->outputQueue.size() <<endl;
      	//cout << "[Scheduler] - State  Queue Size: "<< agent->stateQueue.size() <<endl;
	//cout << "[Scheduler] - Input  Queue Size: "<< agent->inputQueue.size() <<endl;
	//cout << "[Scheduler] - State  timestamp : "<< agent->myState->getTimeStamp() <<endl;
	//cout << "[Scheduler] - eventPQ Size     : "<< agent->eventPQ->size() <<endl;
        agent->doRollbackRecovery(e);
        //cout << "Rollback Recovery Complete\n"<<endl;
        //cout << "Top event at agent now is: " << *agent->eventPQ->top() <<endl;
        //cout << "[Scheduler] - Output Queue Size: "<< agent->outputQueue.size() <<endl;
        //cout << "[Scheduler] - State  Queue Size: "<< agent->stateQueue.size() <<endl;
        //cout << "[Scheduler] - Input  Queue Size: "<< agent->inputQueue.size() <<endl;
        //cout << "[Scheduler] - State  timestamp : "<< agent->myState->getTimeStamp() <<endl;
        //cout << "[Scheduler] - eventPQ Size     : "<< agent->eventPQ->size() <<endl;
        //exit(11);
	
    }
    
    //check if event is an anti-message
    if (e->isAntiMessage() ){
        //since it is an anti-message we don't need to reprocess the event.
        
        //check if this anti-message is for the furture time.
        if (e->getReceiveTime() > agent->LVT) {
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
    
    e->increaseReference();
    agent->eventPQ->push(e);
    return true;
}//end scheduleEvent

Scheduler::~Scheduler(){}//end Scheduler


Time
Scheduler::getNextEventTime() const {
    if (agent_pq.empty()) {
        // The queue is empty. So return infinity.
        return INFINITY;
    }
    // Obtain reference to the top agent in the priority queue.
    const Agent *agent = agent_pq.top();
    //cout << "TOP agent id is: " << agent->getAgentID() << endl;
    //cout << "Agent address: " << 
    // Now, look at the agent's sub-queue to determine top event.
    if (agent->eventPQ->empty()) {
        // No events on the top-most queue.
        //cout << "Agent eventPQ TOp is Empty" <<endl;
        return INFINITY;
        
    }
    //cout << "TOP @ scheduler is: " << *agent->eventPQ->top() << endl;
    // Use the top-agent's top-event.
    return agent->eventPQ->top()->getReceiveTime();
}

#endif
