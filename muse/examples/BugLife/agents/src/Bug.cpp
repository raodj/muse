#ifndef _BUG_CPP
#define	_BUG_CPP

#include "Bug.h"

using namespace std;

Bug::Bug(AgentID id, State* state, CoordAgentIDMap * coords) : Agent(id,state), coord_map(coords){}

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

