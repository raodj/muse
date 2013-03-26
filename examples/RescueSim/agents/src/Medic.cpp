
#ifndef Medic_CPP
#define Medic_CPP

#include "Medic.h"

Medic::Medic(AgentID id, State* state) : Agent(id,state){
    //insert ctor code here
}//end ctor

void
Medic::initialize() throw (std::exception){
    //insert your init code here
}//end initialize

void
Medic::executeTask(const EventContainer* events){
    //if you want you can uncomment the following code for event processing
    //EventContainer::const_iterator it = events->begin();
    //for (; it != events->end(); it++){
    //  Event * current_event = (*it);
    //....do something with current_event here......
    //}
}//end executeTask

void
Medic::finalize(){
    //insert final code here
}//end finalize

Medic::~Medic(){
    //insert dtor code here
}//end dtor

#endif /* Medic_CPP */
