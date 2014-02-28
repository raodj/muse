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
    
*/

#include "PHOLDAgent.h"
#include "Event.h"
#include "MTRandom.h"
#include "Simulation.h"
#include <cstdlib>
#include <cstdio>

using namespace std;
using namespace muse;

PHOLDAgent::PHOLDAgent(AgentID id, PholdState* state, int x, int y, int n, int d) : 
    Agent(id,state), X(x),Y(y), N(n), Delay(d){}

void
PHOLDAgent::initialize() throw (std::exception){
    //we generate N events with random receive times to self
    for (int i = 0; i < N; i++){
        //const int RndDelay = (int)(MTRandom::RandDouble()*Delay);
        //const int RndDelay = (int)(rand() % Delay);
        Time  receive(getTime()+1);//+RndDelay);

        if ( receive < Simulation::getSimulator()->getStopTime() ){
            //cout << "INIT Random Receive Time: " <<receive <<endl;
            Event * e = Event::create(getAgentID(),receive); 
            scheduleEvent(e);
        }
    }
}//end initialize

void
PHOLDAgent::executeTask(const EventContainer* events){

    PholdState *my_state = dynamic_cast<PholdState*>(getState());
    //for every event we get we send out one event
    for(size_t i = 0; (i < events->size()); i++){
        //first make a random receive time for the future
        //const int RndDelay = (int)(MTRandom::RandDouble()*Delay);
        //const int RndDelay = (int)(rand() % Delay);
        Time  receive(getTime()+1);//+RndDelay);

        if ( receive < Simulation::getSimulator()->getStopTime() ){
            //now we need to choose which agent to send this event to.  we
            //do this with equal probability for all 4 neighbours and send
            //to one.
        
            const int Change[4] = {-1, -Y, Y, 1};
            // Compute index into the Change array
            
            int index  = my_state->getIndex();
            // Determine the receiver neighbor
            int new_index = (index + 1) % 4;
        
            int receiverAgentID = getAgentID() + Change[index];
            my_state->setIndex(new_index);
            
            if(receiverAgentID < 0) {
                receiverAgentID+=X*Y;
            }
            if(receiverAgentID >= (X*Y)) {
                receiverAgentID -= X*Y;
            }
            ASSERT((receiverAgentID >= 0) && (receiverAgentID < X*Y));
        
            //make event
            Event * e = Event::create(receiverAgentID,receive);
            //cout << "Sending to Agent: "<< receiverAgentID << " Rand: " <<RndDelay <<endl;
            //printf ("Sending to LP: %d @ time %lf\n",receiverAgentID , receive);
            //schedule the event
            scheduleEvent(e);
            //printf("SendTime %lf, RecvTime %lf, SenderID %4d, ReceiverID %4d\n", getTime(), receive, getAgentID(), receiverAgentID);
        }
    }
}//end executeTask

void
PHOLDAgent::finalize() {}//end finalize

#endif 

