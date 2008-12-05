#ifndef _MUSE_SCHEDULER_H_
#include "../include/Scheduler.h"
#endif

Scheduler::Scheduler(){}

bool
Scheduler::addAgentToScheduler(const AgentID & agentID){
    if (schedule[agentID] == NULL) {
         std::cout << "Created an Entry in the scheduler for Agent: " << agentID << std::endl; 
        schedule[agentID] = new EventPQ;
        return true;
    }
    return false;
}

//please make sure to delete the Container when you are done
EventContainer* Scheduler::getNextEvents(const AgentID & agent){
    //quick check, if heap empty, then return NULL
    if (schedule[agent]->empty()) return NULL;
    
    //not sure about this, but the user should delete this pointer
    //when they are done. Maybe we should use Smart pointers here
    
    EventContainer *events = new EventContainer;
    Event *rootEvent = schedule[agent]->top();
    //std::cout << "Scheduler Size: " << schedule[agent]->size() << std::endl;
    //std::cout << "Getting Root Event for receive Time: " << rootEvent->getReceiveTime() << std::endl;

    events->insert(rootEvent);
    schedule[agent]->pop();
    Event *nextEvent = schedule[agent]->top();
    ///now lets get all the events to process
    while(rootEvent->getReceiveTime() == nextEvent->getReceiveTime() ){
        if (schedule[agent]->empty()) break;
        //std::cout << "Scheduler Size In Loop: " << schedule[agent]->size() << std::endl;
        //std::cout << "Getting Event for receive Time: " << nextEvent->getReceiveTime() << std::endl;
        events->insert(nextEvent);
        schedule[agent]->pop();
        nextEvent = schedule[agent]->top();
    }//end while
    
    return events;
}

bool 
Scheduler::scheduleEvent( Event *e){
    //first lets look up the receiver agent
    std::cout << "Scheduler Added event for agent: "<<e->getReceiverAgentID() << std::endl;
    //make sure the recevier agent has an entry
    if (schedule[e->getReceiverAgentID()] == NULL) return false;
    schedule[e->getReceiverAgentID()]->push(e);
    return true;
}

bool 
Scheduler::scheduleEvents( EventContainer *events){
    EventContainer::iterator it;
    for(it=events->begin(); it != events->end(); ++it){
        schedule[(*it)->getReceiverAgentID()]->push((*it));
    }//end for
    return true;
}

Scheduler::~Scheduler(){}