#ifndef _SPACE_CPP
#define	_SPACE_CPP

#include "Space.h"
#include "BugDataTypes.h"
#include "BugEvent.h"
#include "MoveIn.h"
#include "MoveOut.h"
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
        SpaceState * my_state = static_cast<SpaceState*>(myState);
        //we use a switch on the event type
        switch(current_event->getEventType()){
        case MOVE_IN:
            cout << "SPACE Got MOVE IN event from bug: "<<current_event->getSenderAgentID() <<endl;
            MoveIn     * move_in  = static_cast<MoveIn*>(current_event);
            
            if ( my_state->getBugID() == NO_BUG){
                //means we are not holding a bug
                my_state->setBugID(move_in->getSenderAgentID());
                MoveIn * move = new MoveIn(move_in->getSenderAgentID() , getTime()+1, MOVE_IN);
                move->canBugMoveIn = true;
                scheduleEvent(move);
            }else{
                //means we are full and no bugs allowed
                MoveIn * move = new MoveIn(move_in->getSenderAgentID(), getTime()+1, MOVE_IN);
                move->canBugMoveIn = false; //false by default, but just to be clear to reader
                scheduleEvent(move);
            }
            break;
        case MOVE_OUT:
            cout << "SPACE Got MOVE OUT event from bug: "<<current_event->getSenderAgentID() <<endl;
            
            //we dont need to static cast to MoveOut because we all need info from base class
            if (my_state->getBugID() == current_event->getSenderAgentID()){
                //this means that current bug living here wants to move out.
                my_state->setBugID(NO_BUG);
                MoveOut * move_out = new MoveOut(current_event->getSenderAgentID(), getTime()+1, MOVE_OUT);
                scheduleEvent(move_out);
            }
            break;
        case GROW:
            break;
        case EAT:
            break;
        case SCOUT:
            break;
        }
    }//end for
}//end executeTask

void
Space::finalize() {
    
}//end finalize
#endif 

