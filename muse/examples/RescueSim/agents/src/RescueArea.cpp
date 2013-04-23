
#ifndef RescueArea_CPP
#define RescueArea_CPP

#include "RescueArea.h"
#include "RescueAreaState.h"
#include "VolunteerDataTypes.h"
#include "UpdatePositionEvent.h"
#include "UpdateNearbyEvent.h"

RescueArea::RescueArea(AgentID id, State* state) : Agent(id,state) { }

void RescueArea::initialize() throw (std::exception) { }

void RescueArea::executeTask(const EventContainer* events){
   EventContainer::const_iterator it = events->begin();
   for(; it != events->end(); it++) {
      UpdatePositionEvent * curEvent = static_cast<UpdatePositionEvent*>((*it));
      RescueAreaState *my_state = static_cast<RescueAreaState*>(getState());
      switch(curEvent->e_type) {
      case UpdatePositionVictim:
         my_state->addVictim(curEvent->cur_location);
      break;
      case UpdatePositionVolunteer:
      break;
      }
   }
}

void RescueArea::finalize() { }

#endif /* RescueArea_CPP */
