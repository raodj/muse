
#ifndef Volunteer_CPP
#define Volunteer_CPP

#include "Volunteer.h"

Volunteer::Volunteer(AgentID id, State* state) : Agent(id,state){
    //insert ctor code here
}//end ctor

void
Volunteer::initialize() throw (std::exception){
    //insert your init code here
}//end initialize

void
Volunteer::executeTask(const EventContainer* events){
    //if you want you can uncomment the following code for event processing
    //EventContainer::const_iterator it = events->begin();
    //for (; it != events->end(); it++){
    //  Event * current_event = (*it);
    //....do something with current_event here......
    //}
}//end executeTask

void
Volunteer::finalize(){
    //insert final code here
}//end finalize

Volunteer::~Volunteer(){
    //insert dtor code here
}//end dtor

#endif /* Volunteer_CPP */
