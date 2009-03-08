#ifndef _SPACE_CPP
#define	_SPACE_CPP

#include "Space.h"
#include "Event.h"


using namespace std;

Space::Space(AgentID id, State* state) : Agent(id,state){}

void
Space::initialize() throw (std::exception){

}//end initialize

void
Space::executeTask(const EventContainer* events){

}//end executeTask

void
Space::finalize() {
    
}//end finalize
#endif 

