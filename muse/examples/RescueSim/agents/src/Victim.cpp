#ifndef Victim_CPP
#define Victim_CPP

#include "Victim.h"
#include "MTRandom.h"
#include "VictimState.h"
#include "UpdatePositionEvent.h"

Victim::Victim(AgentID id, State* state, int c, int r) 
   : Agent(id,state), cols(c), rows(r), my_location(-1, -1){ }

void Victim::initialize() throw (std::exception){
   int x = (MTRandom::RandDouble()*(cols));
   while(x == cols) x = (MTRandom::RandDouble()*(cols));
   int y = (MTRandom::RandDouble()*(rows));
   while(y == rows) y = (MTRandom::RandDouble()*(rows));
   my_location.first = x;
   my_location.second = y;
   std::cout << "Initial position of Victim " << getAgentID() << ": " 
      << "(" << my_location.first << ", " << my_location.second << ") at time " << getTime() << ".\n";
   UpdatePositionEvent * updatePos = new UpdatePositionEvent((int)(my_location.second/AREA_COL_WIDTH), 
                                                                  getTime()+.001, my_location, UpdatePositionVictim);
   scheduleEvent(updatePos);
}

void Victim::executeTask(const EventContainer* events){ }

void Victim::finalize(){ }

#endif /* Victim_CPP */
