#ifndef SPACE_CPP
#define	SPACE_CPP

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

#include "Space.h"
#include "BugDataTypes.h"
#include "MoveIn.h"
#include "MoveOut.h"
#include "Eat.h"
#include "Scout.h"

#define NO_BUG  muse::AgentID(-1)

Space::Space(muse::AgentID id, muse::State* state) : Agent(id, state) {
    // Nothing else to be done in the constructor
}

void
Space::initialize() throw (std::exception) {
    // Nothing else to be done in this method
} 

void
Space::executeTask(const EventContainer* events) {
    EventContainer::const_iterator it = events->begin();
    for (; (it != events->end()); it++){
        BugEvent *current_event = static_cast<BugEvent*>(*it);
        SpaceState *my_state    = static_cast<SpaceState*>(getState());
        // We use a switch on the event type
        switch(current_event->getEventType()) {
	case MOVE_IN:
	    executeMoveIn(current_event, my_state);
	    break;
	case MOVE_OUT:
	    executeMoveOut(current_event, my_state);
            break;
        case EAT:
	    executeEat(current_event, my_state);
            break;
        case SCOUT:
	    executeScout(current_event, my_state);
	    break;
	default:
	    std::cerr << "Error: Unhandled event event type encountered "
		      << "in Space agent.\n";
        } // End switch
    } // End for
} // End executeTask()

void
Space::finalize() {
    // Nothing else to be done here.
}

void
Space::executeMoveIn(BugEvent* current_event, SpaceState* my_state) {
    MoveIn* move_in  = static_cast<MoveIn*>(current_event);
    if (my_state->getBugID() == NO_BUG) {
	//means we are not holding a bug 
	my_state->setBugID(move_in->getSenderAgentID());
	MoveIn * move = MoveIn::create(move_in->getSenderAgentID() ,
				       getTime()+1);
	move->canBugMoveIn = true;
	scheduleEvent(move);
    } else { 
	//means we are full and no bugs allowed
	MoveIn * move = MoveIn::create(move_in->getSenderAgentID(),
				       getTime()+1);
	move->canBugMoveIn = false;
	scheduleEvent(move);
    }
}

void
Space::executeMoveOut(BugEvent* current_event, SpaceState* my_state) {
    // We dont need to static cast to MoveOut because we all need info
    // from base class
    if (my_state->getBugID() == current_event->getSenderAgentID()){
	//this means that current bug living here wants to move out.
	my_state->setBugID(NO_BUG);
	MoveOut * move_out = MoveOut::create(current_event->getSenderAgentID(),
					     getTime()+1);
	scheduleEvent(move_out);
    }
}

void
Space::executeGrow(BugEvent*, SpaceState*) {
    // This method does not have any tasks to perform in this agent.
}

void
Space::executeEat(BugEvent* current_event, SpaceState* my_state) {
    // Ok, we need to check how much food is in this space We cast to
    // get the eat amount, this is how much food there was in the
    // space.
    Eat* eat_event     = static_cast<Eat*>(current_event);
    //making these vars for readability
    int food_here      = my_state->getFood();
    int bug_eat_amount = eat_event->eatAmount;
    // If we can provide the full eat amount then we do else we give
    // what we have left
    Eat * eat   = Eat::create(current_event->getSenderAgentID(), getTime()+1, EAT);
    if (food_here > bug_eat_amount) {
	// Good news for the bug it gets what it wants
	eat->setEatAmount(bug_eat_amount);
	// Now we need to reduce the food count in our state
	my_state->setFood(food_here - bug_eat_amount);
    } else {
	// This means that we dont have enough for the bug, so we send
	// it what we have and set our food count to zero
	eat->setEatAmount(food_here);
	// Now we need to reduce the food count in our state
	my_state->setFood(0);
    }
    // Now we tell the bug
    scheduleEvent(eat);
}

void
Space::executeScout(BugEvent* current_event, SpaceState* my_state) {
    // Ok here we send the bug info about the space.
    Scout* scout_reply = Scout::create(current_event->getSenderAgentID(),
					getTime() + 1);
    scout_reply->foodCount = my_state->getFood();
    scheduleEvent(scout_reply);
}

#endif
