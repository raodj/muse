#ifndef _BUG_CPP
#define	_BUG_CPP 

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
    
    my_location.first  =x;
    my_location.second =y;
    
    
    //now make the move in request to the space.
    Time random_move_time = getTime() + 1 + (int)(MTRandom::RandDouble() * 4);
    MoveIn * move = MoveIn::create(coord_map[my_location],random_move_time , MOVE_IN);
    scheduleEvent(move);
   
}//end initialize

void
Bug::executeTask(const EventContainer* events){
    EventContainer::const_iterator it = events->begin();
    BugState   * my_state      = static_cast<BugState*>(getState()); 
    for (; it != events->end(); it++){
        BugEvent   * current_event = static_cast<BugEvent*>((*it)); 
        
        //we use a switch on the event type
        switch(current_event->getEventType()){
            
        case MOVE_IN:
	    {
            MoveIn     * move_in  = static_cast<MoveIn*>(current_event);
            if (move_in->canBugMoveIn){
                //means that the space accepted the bug
                my_state->setLocation(my_location);

                //lets see how much we can eat!!
                Eat * eat   = Eat::create(current_event->getSenderAgentID(), getTime()+1, EAT);
                //here we figure out how much we can eat before death size is reached
                //death size being 10
                int eat_amount = MAX_BUG_SIZE-my_state->getSize();
                eat->setEatAmount(eat_amount);
                scheduleEvent(eat);
                
            }else{
                //with equal probability see which neighbor space the bug moves to?
                int x =  (MTRandom::RandDouble()*(cols));
                //if x was set from random then we make sure we dont get x out of range
                while (x == cols) x =  (MTRandom::RandDouble()*(cols));
                int y =  (MTRandom::RandDouble()*(rows));
                //if y was set from random then we make sure we dont get y out of range 
                while (y == rows) y =  (MTRandom::RandDouble()*(rows));
                
                //now set my_location and ask the space to move in
                my_location.first =x; 
                my_location.second=y; 
                //now make the move in request to the space.
                Time random_move_time = 1+getTime()+(int)(MTRandom::RandDouble()*(5));
                MoveIn *move = MoveIn::create(coord_map[my_location], random_move_time, MOVE_IN);
                scheduleEvent(move);
            }
             
            break;
	    }
        case MOVE_OUT:
	    {
            /** This was for version 10 and below
            //with equal probability see which neighbor space the bug moves to?
            int x = my_state->getLocation().first; 
            int y = my_state->getLocation().second;  
           
            double r=MTRandom::RandDouble();  
            //we do the mod operator because the space wraps around at the edges!
            if(r<0.25)       x = true_mod(x-=1,cols);
            else if(r<0.5)   y = true_mod(y-=1,rows);
            else if(r<0.75)  y = true_mod(y+=1,rows);
            else             x = true_mod(x+=1,cols);

            //now set my_location and ask the space to move in
            my_location.first =x;
            my_location.second=y;
            
            
            //now make the move in request to the space.
            Time random_move_time = 1+getTime()+(int)(MTRandom::RandDouble()*(5));
            MoveIn *move = new MoveIn(coord_map[my_location],random_move_time , MOVE_IN);
            */

            //for version 11 and above, we simple ask what the best scout space returned is :-)
            Time random_move_time = 0.1+getTime()+(int)(MTRandom::RandDouble()*(5));
            cout << "About to Send Move In to space: " << my_state->getScoutSpace().first <<endl;
            MoveIn *move = MoveIn::create(my_state->getScoutSpace().first,random_move_time , MOVE_IN);
            scheduleEvent(move);
            cout << "Send Move In" <<endl;
            break;
	    }
        case GROW:
	    {
            //we cast it to get the size of growth
            Grow     * grow  = static_cast<Grow*>(current_event); 
            int growth = my_state->getSize()+grow->size;
            my_state->setSize(growth);
            
            break;
	    }
        case EAT:
	    {
            //we cast to get the eat amount, this is how much food there was in the space.
            Eat * eat   = static_cast<Eat*>(current_event);

            //here we check if there was any food to eat, if so we grow and try to eat again.
            //otherwise we move out of the space.
            if (eat->eatAmount > 0){
                //now lets send ourself a grow event
                Grow * grow_event = Grow::create(getAgentID(), getTime()+1, GROW);
                grow_event->setSize(eat->eatAmount);
                scheduleEvent(grow_event);

                //let try to eat again!!!
                Eat * eat_again   = Eat::create(current_event->getSenderAgentID(), getTime()+1, EAT);
                //here we figure out how much we can eat before death size is reached
                //death size being 10
                int eat_amount = MAX_BUG_SIZE-my_state->getSize();
                eat_again->setEatAmount(eat_amount);
                scheduleEvent(eat_again);
            }else{
                //we need to send scouts to all four neighbors
                coord c1(true_mod(my_state->getLocation().first-1,cols),
			 my_state->getLocation().second);
                coord c2(true_mod(my_state->getLocation().first+1,cols),
			 my_state->getLocation().second);
                coord c3(my_state->getLocation().first ,
			 true_mod(my_state->getLocation().second-1,rows));
                coord c4(my_state->getLocation().first ,
			 true_mod(my_state->getLocation().second+1,rows));

                cout << "current location:(" <<my_state->getLocation().first<<"," <<
                    my_state->getLocation().second << ")" << endl;
                cout << "c1:(" <<c1.first<<"," <<c1.second << ")" << endl;
                cout << "c2:(" <<c2.first<<"," <<c2.second << ")" << endl;
                cout << "c3:(" <<c3.first<<"," <<c3.second << ")" << endl;
                cout << "c4:(" <<c4.first<<"," <<c4.second << ")" << endl;
                
                Scout * s1 = Scout::create(coord_map[c1], getTime()+1);
                scheduleEvent(s1);
                Scout * s2 = Scout::create(coord_map[c2], getTime()+1);
                scheduleEvent(s2);
                Scout * s3 = Scout::create(coord_map[c3], getTime()+1);
                scheduleEvent(s3);
                Scout * s4 = Scout::create(coord_map[c4], getTime()+1);
                scheduleEvent(s4);
            }
            break;
	    }
        case SCOUT:
	    {
            Scout * scout_reply = static_cast<Scout*>(current_event);
            //first we increase the number of scouts returned.
            int scouts_returned =  my_state->getScoutReturned()+1;

            //check to see if this scout is better then one on record
            if (scout_reply->foodCount >= my_state->getScoutSpace().second ){
                pair<AgentID,int> better_space(scout_reply->getSenderAgentID(),scout_reply->foodCount);
                my_state->setScoutSpace(better_space);
            }
            //check if we gotten all scouts from 4 neighbors
            if (scouts_returned == 4) {
                //we reset the scout returned counter to zero
                my_state->setScoutReturned(0);
               
                 //lets send a move out event, because we are going to move to another space
                MoveOut * move_out = MoveOut::create(coord_map[my_state->getLocation()] , getTime()+1, MOVE_OUT);
                scheduleEvent(move_out);
               
            }else{
                //lastly we increase scouts returned coutner
                my_state->setScoutReturned(scouts_returned);
                //also wait for remaining scouts to return
            }
            
            break;
	    }
        }
    }//end for
    
    cout << *this << endl;
}//end executeTask

void
Bug::finalize() {
    
}//end finalize

ostream&
operator<<(ostream& os, const Bug& bug) {
    BugState   * my_state      = static_cast<BugState*>(bug.getState()); 
    os << "Bug[id="       << bug.getAgentID()                << ","
       << "location=("    << my_state->getLocation().first << "." <<my_state->getLocation().second << "),"
       << "size="         << my_state->getSize()          << "] @ time " << bug.getTime();
    
    return os;
}
#endif 

