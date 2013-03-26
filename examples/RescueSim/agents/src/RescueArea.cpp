
#ifndef RescueArea_CPP
#define RescueArea_CPP

#include "RescueArea.h"
#include "PlayerEvent.h"

RescueArea::RescueArea(AgentID id, State* state) : Agent(id,state){ }//end ctor

void RescueArea::initialize() throw (std::exception){ }//end initialize

void RescueArea::executeTask(const EventContainer* events){
   EventContainer::const_iterator it = events->begin();
   for(; it != events->end(); it++) {
      PlayerEvent * current_event = static_cast<PlayerEvent*>((*it));
      switch(current_event->getEventType()) {

      }
   }
}//end executeTask

void RescueArea::finalize(){ }//end finalize

RescueArea::~RescueArea(){ }//end dtor

#endif /* RescueArea_CPP */
