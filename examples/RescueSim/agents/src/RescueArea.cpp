#ifndef RESCUE_AREA_CPP
#define RESCUE_AREA_CPP

#include "RescueArea.h"
#include "RescueAreaState.h"
#include "UpdatePositionEvent.h"
#include "UpdateNearbyEvent.h"

RescueArea::RescueArea(muse::AgentID id, muse::State* state) :
    Agent(id,state) { }

void
RescueArea::initialize() { }

int
RescueArea::eventItemCount(const size_t itemCount, int startPos,
			   const size_t maxItems) const {
    return std::max(0, std::min(int(itemCount) - startPos, int(maxItems)));
}

void
RescueArea::scheduleNearbyEvent(const muse::AgentID receiver,
                                const std::vector<muse::AgentID>& nearbyVols,
                                const std::vector<coord>& nearbyVics,
                                const bool isLastCol) {
    const int MaxPos = std::max(nearbyVols.size(), nearbyVics.size());
    for(int startPos = 0; startPos < MaxPos; startPos += MAX_EVENT_ARRAY_SIZE) {
	const int numVols = eventItemCount(nearbyVols.size(), startPos);
	const int numVics = eventItemCount(nearbyVics.size(), startPos);
	UpdateNearbyEvent* updateNear =
	    UpdateNearbyEvent::create(receiver, getTime() + 0.01);
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
RescueArea::executeTask(const muse::EventContainer& events){
    UpdatePositionEvent *curEvent = NULL;
    for (muse::Event *it : events) {
	RescueEvent* const current_event = static_cast<RescueEvent*>(it);
	RescueAreaState* const my_state  = static_cast<RescueAreaState*>(getState());
	if(current_event->getEventType() == UpdatePositionVolunteer) {
	    curEvent = static_cast<UpdatePositionEvent*>(current_event);
	    const coord loc            = curEvent->getCurrentLocation();
	    const muse::AgentID sender = current_event->getSenderAgentID();
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
RescueArea::finalize() {
    // Nothing else to be done for now.
}

#endif
