
#ifndef Victim_CPP
#define Victim_CPP

#include "Victim.h"

Victim::Victim(AgentID id, State* state) : Agent(id,state){
    //insert ctor code here
}//end ctor

void
Victim::initialize() throw (std::exception){
    //insert your init code here
}//end initialize

void
Victim::executeTask(const EventContainer* events){
    //if you want you can uncomment the following code for event processing
    //EventContainer::const_iterator it = events->begin();
    //for (; it != events->end(); it++){
    //  Event * current_event = (*it);
    //....do something with current_event here......
    //}
}//end executeTask

void
Victim::finalize(){
    //insert final code here
}//end finalize

Victim::~Victim(){
    //insert dtor code here
}//end dtor

#endif /* Victim_CPP */
