#ifndef _BUG_CPP
#define	_BUG_CPP

#include "Bug.h"
#include "Event.h"

using namespace std;

Bug::Bug(AgentID& id, State* state) : Agent(id,state){}

void
Bug::initialize() throw (std::exception){

}//end initialize

void
Bug::executeTask(const EventContainer* events){

}//end executeTask

void
Bug::finalize() {
    
}//end finalize
#endif 

