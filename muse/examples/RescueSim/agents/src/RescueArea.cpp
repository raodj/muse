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
   UpdatePositionEvent *curEvent = NULL;
   UpdateNearbyEvent *updateNear = NULL;
   for(EventContainer::const_iterator it = events->begin(); it != events->end(); it++) {
      RescueEvent *current_event = static_cast<RescueEvent*>((*it));
      AgentID sender = (int) current_event->getSenderAgentID();
      RescueAreaState *my_state = static_cast<RescueAreaState*>(getState());
      if(current_event->getEventType() == UpdatePositionVolunteer) {
         curEvent = static_cast<UpdatePositionEvent*>(current_event);
         my_state->updateVolunteerPosition(sender, curEvent->getCurrentLocation());
         updateNear = UpdateNearbyEvent::create(sender, getTime()+0.01, UpdateNearby);
         updateNear->setNearbyVols(my_state->getNearbyVolunteers(sender));
         updateNear->setNearbyVics(my_state->getNearbyVictims(sender));
         scheduleEvent(updateNear);
      }
      else if(current_event->getEventType() == UpdatePositionVictim) {
         curEvent = static_cast<UpdatePositionEvent*>(current_event);
         my_state->addVictim(curEvent->getCurrentLocation());
      }
   }
}

void RescueArea::finalize() { }

#endif /* RescueArea_CPP */