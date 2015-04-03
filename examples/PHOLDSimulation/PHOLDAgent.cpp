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

PHOLDAgent::PHOLDAgent(AgentID id, PholdState* state, int x, int y,
                       int n, int d, int lookAhead, double selfEvents,
                       size_t granularity) : 
    Agent(id, state), X(x),Y(y), N(n), Delay(d), lookAhead(lookAhead),
    selfEvents(selfEvents), granularity(granularity) {
    // Setup the random seed used for generating random delays.
    seed = id;
}

void
PHOLDAgent::initialize() throw (std::exception){
    // We generate N events with random receive times to self
    for (int i = 0; i < N; i++){
        const int RndDelay = (Delay > 0) ? ((int) (rand_r(&seed) % Delay)) : 0;
        Time  receive(getTime() + 1 + RndDelay);
        ASSERT( receive > getTime() );
        if ( receive < Simulation::getSimulator()->getStopTime() ){
            Event * e = Event::create(getAgentID(),receive); 
            scheduleEvent(e);
        }
    }
} // End initialize

#pragma GCC push_options
#pragma GCC optimize ("-O0")
void
PHOLDAgent::simGranularity() {
    for (size_t i = 0; (i < granularity); i++) {
        double sum = 0;
        for (size_t delay = 0; (delay < 25L); delay++) {
            sum += sin(0.5);
        }
    }
}
#pragma GCC pop_options

void
PHOLDAgent::executeTask(const EventContainer* events) {
    PholdState *my_state = dynamic_cast<PholdState*>(getState());
    // For every event we get we send out one event
    for (size_t i = 0; (i < events->size()); i++) {
        // Simulate some event granularity
        simGranularity();
        // First make a random receive time for the future
        const int RndDelay = (Delay > 0) ? ((int)(rand_r(&seed) % Delay)) : 0;
        const Time  receiveTime(getTime() + lookAhead + RndDelay);

        if ( receiveTime < Simulation::getSimulator()->getStopTime() ){
            // Now we need to choose the agent to send this event to.
            int receiverAgentID = getAgentID();
            if (selfEvents < 0) {
                receiverAgentID = (receiverAgentID + i) % (X * Y);
            }
            if ((selfEvents >= 0) && (selfEvents < 1) &&
                (((rand_r(&seed) % 1000) / 1000.0) > selfEvents)) {
                // Schedule event to different agent.  We do this with
                // equal probability to choose 1 of 4 neighbours.
                const int Change[4] = {-1, -Y, Y, 1};
                // Compute index into the Change array
                int index  = my_state->getIndex();
                // Determine the receiver neighbor
                int new_index = (index + 1) % 4;
                // Update the destination agent.
                receiverAgentID = getAgentID() + Change[index];
                my_state->setIndex(new_index);
                // Handle wrap around cases.
                if(receiverAgentID < 0) {
                    receiverAgentID += X * Y;
                }
                if(receiverAgentID >= (X * Y)) {
                    receiverAgentID -= X * Y;
                }
            }
            ASSERT((receiverAgentID >= 0) && (receiverAgentID < X * Y));
            // Make event
            Event * e = Event::create(receiverAgentID, receiveTime);
            // Schedule the event
            scheduleEvent(e);
        }
    }
} // End executeTask

void
PHOLDAgent::finalize() {}

#endif 

