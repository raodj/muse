#ifndef _PHOLDAgent_CPP
#define	_PHOLDAgent_CPP

/** The P-HOLD Simulation.
    BY: Meseret R. Gebre         meseret.gebre@muohio.edu

    Used Variables
    **************************
    X = Number of agents per row
    Y = Number of agents per column
    N = Number of initial events each agent starts with
    Delay = The Delay time for the receive time, this will be max range for a random from [0,1]

    MTRandom.h
    **************************
    The interface for MTRandom class is very simple. There are two static methods and two object methods.

    MTRandom::Rand() returns random generated number uniformly distributed in the range 0 - (2^32 - 1)
    MTRandom::RandDouble() returns MTRandom::Rand() and scales it to the range 0 - 1
    MTRandom::rand() is just like MTRandom::Rand(), but only for instances of MTRandom.
    MTRandom::randDouble() is just like MTRandom::RandDouble(), but only for instances of MTRandom.

    Constructing MTRandom
    ***************************
    MTRandom mt; //this version is default and will use a hardcoded seed.
    MTRandom mt(unsigned long seed); //this created with the given seed value.
*/


#include "PHOLDAgent.h"
#include "Event.h"
#include "MTRandom.h"
#include "Simulation.h"

using namespace std;

PHOLDAgent::PHOLDAgent(AgentID id, State* state, int x, int y, int n, int d) : 
    Agent(id,state), X(x),Y(y), N(n), Delay(d){}

void
PHOLDAgent::initialize() throw (std::exception){
    //we generate N events with random receive times to self
    for (int i = 0; i < N; i++){
        const int RndDelay = (int)(MTRandom::RandDouble()*Delay);
        Time  receive(getTime()+1+RndDelay);

        if ( receive < Simulation::getSimulator()->getStopTime() ){
            //cout << "INIT Random Receive Time: " <<receive <<endl;
            Event * e = Event::create(getAgentID(),receive); 
            scheduleEvent(e);
        }
    }
}//end initialize

void
PHOLDAgent::executeTask(const EventContainer* events){
   
    //for every event we get we send out one event
    for(size_t i = 0; (i < events->size()); i++){
        //first make a random receive time for the future
        const int RndDelay = (int)(MTRandom::RandDouble()*Delay);
        Time  receive(getTime()+1+RndDelay);

        if ( receive < Simulation::getSimulator()->getStopTime() ){
            //now we need to choose which agent to send this event to.  we
            //do this with equal probability for all 4 neighbours and send
            //to one.
        
            const int Change[4] = {-1, -Y, Y, 1};
            // Compute index into the Change array
            int index = (int) (MTRandom::RandDouble() * 4);
            // Determine the receiver neighbor
            int receiverAgentID = getAgentID() + Change[index];

            if(receiverAgentID < 0) {
                receiverAgentID+=X*Y;
            }
            if(receiverAgentID >= (X*Y)) {
                receiverAgentID -= X*Y;
            }
            ASSERT((receiverAgentID >= 0) && (receiverAgentID < X*Y));
        
            //make event
            Event * e = Event::create(receiverAgentID,receive);
            
            //schedule the event
            scheduleEvent(e);
        }
    }
}//end executeTask

void
PHOLDAgent::finalize() {}//end finalize

#endif 

