#ifndef _SPACE_CPP
#define	_SPACE_CPP

#include "Space.h"
#include "BugDataTypes.h"
#include "BugEvent.h"
#include "MoveIn.h"
#include "MoveOut.h"
#include "Eat.h"
#include "Scout.h"
#include "SpaceState.h"

using namespace std;

#define NO_BUG  -1u

Space::Space(AgentID id, State* state) : Agent(id,state){}

void
Space::initialize() throw (std::exception){

}//end initialize

void
Space::executeTask(const EventContainer* events){
    EventContainer::const_iterator it = events->begin();
    
    for (; it != events->end(); it++){
        BugEvent * current_event = static_cast<BugEvent*>((*it));
        SpaceState * my_state = static_cast<SpaceState*>(getState());
        //we use a switch on the event type
        switch(current_event->getEventType()){
        case MOVE_IN:
	    {   
            MoveIn     * move_in  = static_cast<MoveIn*>(current_event);
             
            if ( my_state->getBugID() == NO_BUG){
                
                //means we are not holding a bug 
                my_state->setBugID(move_in->getSenderAgentID());
                MoveIn * move = MoveIn::create(move_in->getSenderAgentID() , getTime()+1, MOVE_IN);
                move->canBugMoveIn = true;
                scheduleEvent(move);
                  
                
            }else{ 
                //means we are full and no bugs allowed
                MoveIn * move = MoveIn::create(move_in->getSenderAgentID(), getTime()+1, MOVE_IN);
                move->canBugMoveIn = false; //false by default, but just to be clear to reader
                scheduleEvent(move);
            }
            break;
	}
        case MOVE_OUT:
	    {
            //we dont need to static cast to MoveOut because we all need info from base class
            if (my_state->getBugID() == current_event->getSenderAgentID()){
                //this means that current bug living here wants to move out.
                my_state->setBugID(NO_BUG);
                MoveOut * move_out = MoveOut::create(current_event->getSenderAgentID(), getTime()+1, MOVE_OUT);
                scheduleEvent(move_out);
            }
            break;
            }
        case EAT:
	    {  
            //ok, we need to check how much food is in this space
            //we cast to get the eat amount, this is how much food there was in the space.
            Eat * eat_event           = static_cast<Eat*>(current_event);
            //making these vars for readability
            int food_here       = my_state->getFood();
            int bug_eat_amount  = eat_event->eatAmount;
            //if we can provide the full eat amount then we do else we give what we have left
            Eat * eat   = Eat::create(current_event->getSenderAgentID(), getTime()+1, EAT);
            if (food_here > bug_eat_amount) {
                //good news for the bug it gets what it wants
                eat->setEatAmount(bug_eat_amount);
                //now we need to reduce the food count in our state
                my_state->setFood( (food_here-bug_eat_amount) );
            }else{
                //this means that we dont have enough for the bug, so we send him what we have
                //and set our food count to zero
                eat->setEatAmount(food_here);
                //now we need to reduce the food count in our state
                my_state->setFood(0);
            }
            //now we tell the bug
            scheduleEvent(eat);
            break;
            }
        case SCOUT:
            //ok here we send the bug info about the space.
            //cout << "space got scout"<<endl;
            Scout * scout_reply = Scout::create(current_event->getSenderAgentID(), getTime()+1, SCOUT);
            scout_reply->foodCount = my_state->getFood();
            scheduleEvent(scout_reply);
            break;
        }
    }//end for
}//end executeTask

void
Space::finalize() {
    
}//end finalize
#endif 

