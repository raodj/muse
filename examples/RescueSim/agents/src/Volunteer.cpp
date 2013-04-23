
#ifndef Volunteer_CPP
#define Volunteer_CPP

#include "Volunteer.h"
#include "VolunteerDataTypes.h"
#include "VolunteerEvent.h"
#include "VolunteerState.h"
#include "UpdatePositionEvent.h"

Volunteer::Volunteer(AgentID id, State* state, CoordAgentIDMap * coords, int c, int r , int x, int y)
   : Agent(id, state), cols(c), rows(r), my_location(x, y) {
      coord_map = *coords;
}

void Volunteer::initialize() throw (std::exception) { }

void Volunteer::executeTask(const EventContainer* events){
   EventContainer::const_iterator it = events->begin();
   for (; it != events->end(); it++){
      Event * current_event = (*it);
   }
}

void Volunteer::finalize(){ }

#endif /* Volunteer_CPP */
