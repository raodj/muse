#ifndef BUG_CPP
#define	BUG_CPP 

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "Bug.h"
#include "MTRandom.h"
#include <iostream>
#include "BugDataTypes.h" 
#include "MoveIn.h"
#include "MoveOut.h" 
#include "Grow.h"
#include "Eat.h"
#include "Scout.h"
#include "BugEvent.h"
#include "BugState.h"

Bug::Bug(muse::AgentID id, CoordAgentIDMap& coords, int cols, int rows) :
    Agent(id, new BugState()), coord_map(coords), cols(cols), rows(rows),
    my_location(-1,-1) {
    // Nothing else to be done in the constructor.
}

void
Bug::initialize() throw (std::exception) {
}

int
Bug::getRandom(const int max) const {
    int rnd =  int(MTRandom::RandDouble()* max);
    // Make sure we dont get out of range random value
    while (rnd == max)  {
	rnd = int(MTRandom::RandDouble() * max);
    }
    return max;
}

void
Bug::scheduleMoveInEvent(const int timeFactor) {
    // Setup initial location.  This value is not in the state because
    // the bug is still logically not at this location yet. Instead an
    // event is scheduled in the future for the bug to move to this
    // location if the Eco (Space) area accepts the bug.
    my_location.first  = getRandom(cols);
    my_location.second = getRandom(rows);
    
    // Now make the move in request to the space.
    Time moveTime = getTime() + 1 + getRandom(timeFactor);
    MoveIn * moveIn = MoveIn::create(coord_map.at(my_location), moveTime);
    scheduleEvent(moveIn);
}

void
Bug::executeTask(const EventContainer* events) {
    EventContainer::const_iterator it = events->begin();
    BugState&  my_state               = *static_cast<BugState*>(getState()); 
    for (; (it != events->end()); it++) {
        BugEvent* const current_event = static_cast<BugEvent*>(*it); 
	// Based on the event type, the corresponding method is called
	// with 'current_event' and 'my_state' passed as arguments.
        switch(current_event->getEventType()) {
	case MOVE_IN:
	    executeMoveIn(current_event, my_state);
	    break;
	case MOVE_OUT:
	    executeMoveOut(current_event, my_state);
	    break;
	case GROW:
	    executeGrow(current_event, my_state);
	    break;
	case EAT:
	    executeEat(current_event, my_state);
	    break;
	case SCOUT:
	    executeScout(current_event, my_state);
	    break;
	default:
	    std::cerr << "Unhandled event type encountered in Bug.cpp\n";
	    ASSERT( false );
        }
    }
}

void
Bug::finalize() {
    // Nothing else to be done for now in finalize
}

void
Bug::executeMoveIn(BugEvent* currentEvent, BugState& state) {
    MoveIn* moveInEvent = static_cast<MoveIn*>(currentEvent);
    if (moveInEvent->canBugMoveIn) {
	// Means that the space accepted the bug
	state.setLocation(my_location);
	// Lets see how much we can eat!!
	Eat* eat = Eat::create(moveInEvent->getSenderAgentID(), getTime() + 1);
	// Here we figure out how much we can eat before death size is
	// reached death size being 10
	int eatAmount = MAX_BUG_SIZE - state.getSize();
	eat->setEatAmount(eatAmount);
	scheduleEvent(eat);
    } else {
	// With equal probability see which neighbor space the bug
	// moves to.  There the timeFactor is 5 while initial value
	// was 4.  Not sure if this difference is meaningful.
	scheduleMoveInEvent(5);
    }
}

void
Bug::executeMoveOut(BugEvent*, BugState& state) {
    Time moveTime = 1 + getTime() + getRandom(5);
    MoveIn *move = MoveIn::create(state.getScoutSpace().first, moveTime);
    scheduleEvent(move);
}

void Bug::executeGrow(BugEvent* event, BugState& state) {
    // We cast it to get the size of growth
    Grow* grow = static_cast<Grow*>(event); 
    int growth = state.getSize() + grow->size;
    state.setSize(growth);
}

void
Bug::executeEat(BugEvent* event, BugState& state) {
    // We cast to get the eat amount. The event contains how much food
    // there was in the space for this bug to eat
    Eat* eatEvent = static_cast<Eat*>(event);
    // Here we check if there was any food to eat, if so we grow and
    // try to eat again.  otherwise we move out of the space.
    if (eatEvent->eatAmount > 0) {
	// Now lets send ourself a grow event
	Grow* growEvent = Grow::create(getAgentID(), getTime() + 1);
	growEvent->setSize(eatEvent->eatAmount);
	scheduleEvent(growEvent);
	
	// Lets try to eat again!!!
	Eat* eatMore = Eat::create(eatEvent->getSenderAgentID(), getTime() + 1);
	// Here we figure out how much we can eat before death size is
	// reached death size being 10
	eatMore->setEatAmount(MAX_BUG_SIZE - state.getSize());
	scheduleEvent(eatMore);
    } else {
	// We need to send scouts to all four neighbors. The following
	// array has pairs of values to modify rows and columns to
	// determine adjacent neighbor coordinates.
	const int Offset[]  = {-1, 0, 1, 0, 0, -1, 0, 1};
	const coord currPos = state.getLocation();
	for(int i = 0; (i < 8); i += 2) {
	    coord c1(true_mod(currPos.first  + Offset[i + 0], cols),
		     true_mod(currPos.second + Offset[i + 1], rows));
	    Scout* s1 = Scout::create(coord_map.at(c1), getTime() + 1);
	    scheduleEvent(s1);
	}
    }
}

void
Bug::executeScout(BugEvent* current_event, BugState& my_state) {
    Scout * scout_reply = static_cast<Scout*>(current_event);
    // First we increase the number of scouts returned.
    int scouts_returned =  my_state.getScoutReturned()+1;

    // Check to see if this scout is better then one on record
    if (scout_reply->foodCount >= my_state.getScoutSpace().second) {
	std::pair<AgentID, int> better_space(scout_reply->getSenderAgentID(),
					     scout_reply->foodCount);
	my_state.setScoutSpace(better_space);
    }
    // Check if we have received all scouts from 4 neighbors
    if (scouts_returned == 4) {
	//we reset the scout returned counter to zero
	my_state.setScoutReturned(0);
	   
	// Lets send a move out event, because we are going to move to
	// another space
	AgentID spaceAgent = coord_map.at(my_state.getLocation());
	MoveOut* move_out  = MoveOut::create(spaceAgent, getTime() + 1);
	scheduleEvent(move_out);
    } else {
	// Lastly we increase scouts returned counter. 
	my_state.setScoutReturned(scouts_returned);
    }
}

ostream& operator<<(ostream& os, const Bug& bug) {
    const BugState* my_state = static_cast<BugState*>(bug.getState());
    os << "Bug[id="    << bug.getAgentID() << ","
       << "location=(" << my_state->getLocation().first << ", "
       << my_state->getLocation().second << "), "
       << "size="      << my_state->getSize() << "] @ time " << bug.getTime();
    return os;
}

#endif 
