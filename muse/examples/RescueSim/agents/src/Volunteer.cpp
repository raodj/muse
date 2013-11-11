#ifndef Volunteer_CPP
#define Volunteer_CPP

#include "Volunteer.h"
#include "VolunteerDataTypes.h"
#include "VolunteerEvent.h"
#include "RescueEvent.h"
#include "VolunteerState.h"
#include "UpdatePositionEvent.h"
#include "UpdateNearbyEvent.h"
#include <stdlib.h>

Volunteer::Volunteer(AgentID id, State* state, int c, int r , int x, int y)
   : Agent(id, state), cols(c), rows(r), my_location(x, y) { }

void Volunteer::initialize() throw (std::exception) {
   (static_cast<VolunteerState*>(getState()))->setLocation(my_location);
   (static_cast<VolunteerState*>(getState()))->setAgentID(getAgentID());
   for(int i = 0; i < 8; i++) (static_cast<VolunteerState*>(getState()))->getMoveTracker()[i] = 3;
   std::cout << "Initial position of Voluneeter " << getAgentID() << ": " 
      << "(" << my_location.first << ", " << my_location.second << ") at time " << getTime() << ".\n";
   calculateMove();
   UpdatePositionEvent * updatePos = UpdatePositionEvent::create((int)(my_location.second/AREA_COL_WIDTH), 
                                                                  getTime()+1, my_location, UpdatePositionVolunteer);
   scheduleEvent(updatePos);
}

void Volunteer::executeTask(const EventContainer* events){
   UpdateNearbyEvent* upEvent = NULL;
   VolunteerEvent* volEvent = NULL;
   for(EventContainer::const_iterator it = events->begin(); it != events->end(); it++){
      RescueEvent * current_event = static_cast<RescueEvent*>((*it));
      VolunteerState * my_state = static_cast<VolunteerState*>(getState());
      if(current_event->getEventType() == VolunteerReport) {
         volEvent = static_cast<VolunteerEvent*>(current_event);
         my_state->updateKnownVics(volEvent->getNearbyVics());
      }
      else if(current_event->getEventType() == UpdateNearby) {
         upEvent = static_cast<UpdateNearbyEvent*>(current_event);
         my_state->updateNearbyVols(upEvent->getNearbyVols());
         my_state->updateKnownVics(upEvent->getNearbyVics());
         for(unsigned int i = 0; i < my_state->getNearbyVolunteers().size(); i++) {
            VolunteerEvent * reportEvent = VolunteerEvent::create(my_state->getNearbyVolunteers()[i], 
                                                                     getTime()+0.01, VolunteerReport);
            scheduleEvent(reportEvent);
         }
         calculateMove();
         UpdatePositionEvent * updatePos = UpdatePositionEvent::create((int)(my_state->getLocation().second/AREA_COL_WIDTH), 
                                                         getTime()+1, my_state->getLocation(), UpdatePositionVolunteer);
         scheduleEvent(updatePos);
      }
   }
}

// [0] [1] [2]
// [7]     [3]
// [6] [5] [4]
void Volunteer::calculateMove() {
   VolunteerState * my_state = static_cast<VolunteerState*>(getState());
   coord curLoc = my_state->getLocation();
   int nextMove = 0;
   int probMove[8];
   for(int i = 0; i < 8; i++) if(my_state->getMoveTracker()[i] == 0) my_state->getMoveTracker()[i]++;
   if(curLoc.first == 0) {
      my_state->getMoveTracker()[0] = 0;
      my_state->getMoveTracker()[6] = 0;
      my_state->getMoveTracker()[7] = 0;
   }
   else if(curLoc.first == cols) {
      my_state->getMoveTracker()[2] = 0;
      my_state->getMoveTracker()[3] = 0;
      my_state->getMoveTracker()[4] = 0;
   }
   if(curLoc.second == 0) {
      my_state->getMoveTracker()[0] = 0;
      my_state->getMoveTracker()[1] = 0;
      my_state->getMoveTracker()[2] = 0;
   }
   else if(curLoc.second == rows) {
      my_state->getMoveTracker()[4] = 0;
      my_state->getMoveTracker()[5] = 0;
      my_state->getMoveTracker()[6] = 0;
   }
   for(int i = 0; i < 8; i++) probMove[i] = my_state->getMoveTracker()[i];
   //std::cout << "Before: "; for(int i = 0; i < 8; i++) std::cout << probMove[i] << " "; std::cout << std::endl;
   for(int i = 1; i < 8; i++) probMove[i] = probMove[i-1] + probMove[i];
   int r = rand() % probMove[7];
   for(int i = 0; i < 8; i++) {
      if(r < probMove[i]) break;
      nextMove++;
   }
   if(nextMove <= 2) curLoc.second--;
   if(nextMove >= 2 && nextMove <= 4) curLoc.first++;
   if(nextMove >= 4 && nextMove <= 6) curLoc.second++;
   if(nextMove >= 6 || nextMove == 0) curLoc.first--;
   my_state->getMoveTracker()[nextMove] += 3;
   my_state->getMoveTracker()[(nextMove+1)%8] += 1;
   my_state->getMoveTracker()[(nextMove+7)%8] += 1;
   my_state->getMoveTracker()[(nextMove+4)%8] -= 3;
   my_state->getMoveTracker()[(nextMove+3)%8] -= 1;
   my_state->getMoveTracker()[(nextMove+5)%8] -= 1;
   for(int i = 0; i < 8; i++) if(my_state->getMoveTracker()[i] < 0) my_state->getMoveTracker()[i] = 0;
   my_state->setLocation(curLoc);
   std::cout << "Voluneeter " << getAgentID() << " moves to " << "(" << curLoc.first << ", " 
             << curLoc.second << ") at time " << (getTime()+1) << ".\n";
}

void Volunteer::finalize(){ }

#endif /* Volunteer_CPP */
