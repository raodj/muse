
#ifndef PlayerEvent_CPP
#define PlayerEvent_CPP

#include "PlayerEvent.h"

PlayerEvent::PlayerEvent(AgentID receiver_id, Time receive_time) : Event(receiver_id, receive_time){
    //insert ctor code here
}//end ctor

PlayerEvent::~PlayerEvent(){
    //insert dtor code here
}//end dtor

#endif /* PlayerEvent_CPP */
