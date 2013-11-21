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
         coord loc = curEvent->getCurrentLocation();
         my_state->updateVolunteerPosition(sender, loc);
         updateNear = UpdateNearbyEvent::create(sender, getTime()+0.01, UpdateNearby);
         std::vector<AgentID> nearbyVols = my_state->getNearbyVolunteers(sender);
         std::vector<coord> nearbyVics = my_state->getNearbyVictims(sender);
         unsigned int vo = 0;
         unsigned int vi = 0;
         int numVol = 0;
         int numVic = 0;
         AgentID Vols[MAX_EVENT_ARRAY_SIZE];
         coord Vics[MAX_EVENT_ARRAY_SIZE];
         while(vo < nearbyVols.size() || vi < nearbyVics.size()) {
            if(vo < nearbyVols.size()) {
               Vols[numVol] = nearbyVols[vo];
               vo++;
               numVol++;
            }
            if(vi < nearbyVics.size()) {
               Vics[numVic] = nearbyVics[vi];
               vi++;
               numVic++;
            }
            if(numVic == MAX_EVENT_ARRAY_SIZE || numVol == MAX_EVENT_ARRAY_SIZE) {
               updateNear->setNearbyVols(Vols, numVol);
               updateNear->setNearbyVics(Vics, numVic);
               scheduleEvent(updateNear);
               numVol = 0;
               numVic = 0;
            }
         }
         updateNear->setNearbyVols(Vols, numVol);
         updateNear->setNearbyVics(Vics, numVic);
         if(my_state->getColID() == loc.second/AREA_COL_WIDTH) 
            updateNear->setMessageFinal();
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