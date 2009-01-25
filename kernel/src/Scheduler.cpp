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
#include "Agent.h"

using namespace muse;
Scheduler::Scheduler(){}

bool
Scheduler::addAgentToScheduler(Agent * agent){
    if (agentMap[agent->getAgentID()] == NULL) {
        agentMap[agent->getAgentID()] = agent;
        agent_pq.push(agent);
        return true;
    }
    return false;
}//end addAgentToScheduler

bool
Scheduler::processNextAgentEvents(){
    Agent * agent = agent_pq.top();
    bool result = agent->processNextEvents();
    agent_pq.pop();
    agent_pq.push(agent);
    return result;
}//end processNextAgentEvents

bool Scheduler::scheduleEvent( Event *e){
    //make sure the recevier agent has an entry
    if (agentMap[e->getReceiverAgentID()] == NULL) return false;
    agentMap[e->getReceiverAgentID()]->eventPQ.push(e);
    return true;
}//end scheduleEvent

Scheduler::~Scheduler(){}//end Scheduler

//bool
//Scheduler::operator()(const Agent *lhs, const Agent *rhs) const
//{
//    if (lhs->eventPQ.empty() && !rhs->eventPQ.empty()) {
//        //std::cout << "[CompScheduler] lhs is EMPTY"<< std::endl;
//        return true;
//    }
//    if (rhs->eventPQ.empty() && !lhs->eventPQ.empty()) {
//        //std::cout << "[CompScheduler] rhs is EMPTY"<< std::endl;
//        return false;
//    }
//    Event *lhs_event = lhs->eventPQ.top();
//    Event *rhs_event = rhs->eventPQ.top();
//    Time lhs_time    = lhs_event->getReceiveTime();
//    Time rhs_time    = rhs_event->getReceiveTime();
//    //std::cout << "[CompScheduler] " << lhs_time << " > " << rhs_time << std::endl;
//    return (lhs_time > rhs_time);
//}//end operator()

#endif
