
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
   UpdatePositionEvent * curEvent;
   UpdateNearbyEvent * updateNear;
   for(; it != events->end(); it++) {
      RescueEvent * current_event = static_cast<RescueEvent*>((*it));
      RescueAreaState *my_state = static_cast<RescueAreaState*>(getState());
      switch(current_event->getEventType()) {
      curEvent = static_cast<UpdatePositionEvent*>(current_event);
      case UpdatePositionVolunteer:
         my_state->updateVolunteerPosition(curEvent->getSenderAgentID(), curEvent->cur_location);
         updateNear = new UpdateNearbyEvent(curEvent->getSenderAgentID(), getTime(), UpdateNearby);
         updateNear->setNearbyVols(my_state->getNearbyVolunteers(curEvent->getSenderAgentID()));
         updateNear->setNearbyVics(my_state->getNearbyVictims(curEvent->getSenderAgentID()));
         scheduleEvent(updateNear);
      break;
      case UpdatePositionVictim:
         my_state->addVictim(curEvent->cur_location);
      break;
      default: break;
      }
   }
}

void RescueArea::finalize() { }

#endif /* RescueArea_CPP */
