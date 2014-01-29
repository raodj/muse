#ifndef RescueArea_CPP
#define RescueArea_CPP

#include "RescueArea.h"
#include "RescueAreaState.h"
#include "UpdatePositionEvent.h"
#include "UpdateNearbyEvent.h"

RescueArea::RescueArea(AgentID id, State* state) : Agent(id,state) { }

void
RescueArea::initialize() throw (std::exception) { }

void
RescueArea::scheduleNearbyEvent(const AgentID receiver,
                                const std::vector<AgentID>& nearbyVols,
                                const std::vector<coord>& nearbyVics,
                                const bool isLastCol) {
   const size_t MaxPos = std::max(nearbyVols.size(), nearbyVics.size());
   for(size_t startPos = 0; startPos < MaxPos; startPos += MAX_EVENT_ARRAY_SIZE) {
      const int numVols = std::min(nearbyVols.size() - startPos,
                                   MAX_EVENT_ARRAY_SIZE);
      const int numVics = std::min(nearbyVics.size() - startPos,
                                   MAX_EVENT_ARRAY_SIZE);
      UpdateNearbyEvent* updateNear = UpdateNearbyEvent::create(receiver, 
                                                                getTime() + 0.01);
      updateNear->setNearbyVols(&nearbyVols[startPos], numVols);
      updateNear->setNearbyVics(&nearbyVics[startPos], numVics);
      scheduleEvent(updateNear);
   }
   if(isLastCol) {
      UpdateNearbyEvent* updateNear = UpdateNearbyEvent::create(receiver, 
                                                                getTime() + 0.01);
      updateNear->setMessageFinal();
      scheduleEvent(updateNear);
   }
}

void
RescueArea::executeTask(const EventContainer* events){
   UpdatePositionEvent *curEvent = NULL;
   for(EventContainer::const_iterator it = events->begin();
                                                       it != events->end(); it++) {
      RescueEvent* const current_event = static_cast<RescueEvent*>((*it));
      RescueAreaState* const my_state  = static_cast<RescueAreaState*>(getState());
      if(current_event->getEventType() == UpdatePositionVolunteer) {
         curEvent = static_cast<UpdatePositionEvent*>(current_event);
         const coord loc      = curEvent->getCurrentLocation();
         const AgentID sender = current_event->getSenderAgentID();
         my_state->updateVolunteerPosition(sender, loc);
         scheduleNearbyEvent(current_event->getSenderAgentID(),
                             my_state->getNearbyVolunteers(sender),
                             my_state->getNearbyVictims(sender),
                             (my_state->getColID() == loc.second/AREA_COL_WIDTH));
      }
      else if(current_event->getEventType() == UpdatePositionVictim) {
         curEvent = static_cast<UpdatePositionEvent*>(current_event);
         my_state->addVictim(curEvent->getCurrentLocation());
      }
   }
}

void
RescueArea::finalize() { }

#endif /* RescueArea_CPP */
