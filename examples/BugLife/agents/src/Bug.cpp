#ifndef _BUG_CPP
#define	_BUG_CPP

#include "Bug.h"
#include "MTRandom.h"
#include <iostream>
#include "BugDataTypes.h"
#include "MoveIn.h"
#include "MoveOut.h"
#include "BugEvent.h"
#include "BugState.h"

using namespace std;

Bug::Bug(AgentID id, State* state, CoordAgentIDMap * coords, int c, int r) :
    Agent(id,state), cols(c), rows(r) , my_location(-1,-1){
    coord_map = *coords;
}

void
Bug::initialize() throw (std::exception){
    int x =  (MTRandom::RandDouble()*(cols));
    //make sure we dont get out of range random value
    while (x == cols) x =  (MTRandom::RandDouble()*(cols));
    int y =  (MTRandom::RandDouble()*(rows));
    while (y == rows) y =  (MTRandom::RandDouble()*(rows));
    //cout << "Bug: " << getAgentID() << " moving to ("<<x<<","<<y<<")" <<endl;
    my_location.first =x;
    my_location.second=y;
    //cout << "Trying to Move to Space: " << coord_map[my_location] <<endl;

    //now make the move in request to the space.
    MoveIn * move = new MoveIn(coord_map[my_location], getTime()+1, MOVE_IN);
    scheduleEvent(move);
}//end initialize

void
Bug::executeTask(const EventContainer* events){
    EventContainer::const_iterator it = events->begin();
    for (; it != events->end(); it++){
        BugEvent * current_event = static_cast<BugEvent*>((*it));
        BugState   * my_state = static_cast<BugState*>(myState);
        //we use a switch on the event type
        switch(current_event->getEventType()){
        case MOVE_IN:
            //cout << "BUG Got MOVE IN event" <<endl;
           
            MoveIn     * move_in  = static_cast<MoveIn*>(current_event);
            if (move_in->canBugMoveIn){
                //means that the space accepted the bug
                oss << "Bug " <<getAgentID()<< " moved to ("<<my_location.first<<","<<my_location.second<<") @ time "<<getTime()<< endl;
                my_state->setLocation(my_location);

                //lets send a move out event, because we are going to move to another space
                MoveOut * move_out = new MoveOut(current_event->getSenderAgentID(), getTime()+1, MOVE_OUT);
                scheduleEvent(move_out);
                
            }else{
                
                //with equal probability see which neighbor space the bug moves to?
                int x =  (MTRandom::RandDouble()*(cols));
                //if x was set from random then we make sure we dont get x out of range
                while (x == cols) x =  (MTRandom::RandDouble()*(cols));
                int y =  (MTRandom::RandDouble()*(rows));
                //if y was set from random then we make sure we dont get y out of range
                while (y == rows) y =  (MTRandom::RandDouble()*(rows));
                
                double r=MTRandom::RandDouble();
                //we do the mod operator because the space wraps around at the edges!
                if(r<0.25)      (x--) % cols; 
                else if(r<0.5)  (y--) % rows;
                else if(r<0.75) (y++) % rows;
                else            (x++) % cols;
                
                //now set my_location and ask the space to move in
                my_location.first =x;
                my_location.second=y;
                //now make the move in request to the space.
                MoveIn *move = new MoveIn(coord_map[my_location], getTime()+1, MOVE_IN);
                scheduleEvent(move);
            }
            
            break;
        case MOVE_OUT:
            //with equal probability see which neighbor space the bug moves to?
            int x = my_state->getLocation().first;
            int y = my_state->getLocation().second; 
           
            double r=MTRandom::RandDouble();
            //we do the mod operator because the space wraps around at the edges!
            if(r<0.25)      (x--) % cols; 
            else if(r<0.5)  (y--) % rows;
            else if(r<0.75) (y++) % rows;
            else            (x++) % cols;
            
            //now set my_location and ask the space to move in
            my_location.first =x;
            my_location.second=y;
            //now make the move in request to the space.
            MoveIn *move = new MoveIn(coord_map[my_location], getTime()+1, MOVE_IN);
            scheduleEvent(move);
            
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
Bug::finalize() {
    
}//end finalize
#endif 

