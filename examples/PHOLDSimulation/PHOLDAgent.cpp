#ifndef PHOLD_AGENT_CPP
#define	PHOLD_AGENT_CPP

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

/** The P-HOLD Simulation.
    BY: Meseret R. Gebre         meseret.gebre@muohio.edu

    Used Variables
    **************************
    X = Number of agents per row
    Y = Number of agents per column
    N = Number of initial events each agent starts with
    Delay = The Delay time for the receive time, this will be max range
            for a random from [0,1]
*/

#include "PHOLDAgent.h"
#include "Event.h"
#include "MTRandom.h"
#include "Simulation.h"
#include <cstdlib>
#include <cctype>
#include <random>
#include <algorithm>
#include <map>

using namespace std;
using namespace muse;

// The static maxDelay value
int PHOLDAgent::maxDelay = -1;

PHOLDAgent::PHOLDAgent(AgentID id, PholdState* state, int x, int y,
                       int n, int d, int lookAhead, double selfEvents,
                       size_t granularity, DelayType type) : 
    Agent(id, state), X(x),Y(y), N(n), Delay(d), delayType(type),
    lookAhead(lookAhead),  selfEvents(selfEvents), rng(id),
    granularity(granularity) {
     // Setup the random seed used for generating self/other events
    seed = id;
    // Setup the maximum random delay value for reverse distributions
    if ((Delay > 0) && (maxDelay == -1)) {
        maxDelay = getMaxDelayValue();
        std::cout << "maxDelay = " << maxDelay << std::endl;
    }
}

int
PHOLDAgent::getDelay(const DelayType delType) {
    switch (delType) {
    case UNIFORM: {
        std::uniform_int_distribution<int> uni(0, Delay);
        return uni(rng);
    }
    case POISSON: {
        std::poisson_distribution<int> poi(Delay);
        return poi(rng);
    }
    case EXPONENTIAL: {
        std::exponential_distribution<double> exp(Delay);
        return 2 * exp(rng);
    }
    case REVERSE_POISSON: {
        std::poisson_distribution<int> poi(Delay);
        return maxDelay - (poi(rng) % maxDelay);
    }
    case REVERSE_EXPONENTIAL: {
        std::exponential_distribution<double> exp(Delay);
        return maxDelay - ((int) (2 * exp(rng)) % maxDelay);
    }
    case INVALID_DELAY:
    default:
        return 0;
    }        
}

int
PHOLDAgent::getMaxDelayValue() const {
    int maxDel = Delay;  // default maximum delay that can be generated.
    std::default_random_engine rnd;
    if ((delayType == POISSON) || (delayType == REVERSE_POISSON)) {
        maxDel = 0;
        std::poisson_distribution<int> poi(Delay);
        for (int i = 0; (i < 100000); i++) {
            maxDel = std::max<int>(maxDel, poi(rnd));
        }
    }
    if ((delayType == EXPONENTIAL) || (delayType == REVERSE_EXPONENTIAL)) {
        maxDel = 0;
        std::exponential_distribution<double> poi(Delay);
        for (int i = 0; (i < 100000); i++) {
            // The 2 factor is from http://en.cppreference.com/
            // example.  Since exponential distribution is real/double
            // the 2 factor provides a better spread.
            maxDel = std::max<int>(maxDel, 2 * poi(rnd));
        }        
    }
    return maxDel;
}
    
void
PHOLDAgent::initialize() throw (std::exception){
    // We generate N events with random receive times to self
    for (int i = 0; i < N; i++){
        const int RndDelay = (Delay > 0) ? getDelay(delayType) : 0;
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
        const int RndDelay = (Delay > 0) ? getDelay(delayType) : 0;
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

// Convert delay string to enumeration
PHOLDAgent::DelayType
PHOLDAgent::toDelayType(const std::string& delay) {
    // Convert delay to all lower-case letters
    std::string delay_str(delay);
    std::transform(delay.begin(), delay.end(), delay_str.begin(), ::tolower);
    // Convert string to delays based on their value.
    if ((delay_str == "uniform") || (delay_str == "1")) {
        return UNIFORM;
    } else if ((delay_str == "poisson") || (delay_str == "2")) {
        return POISSON;
    } else if ((delay_str == "exponential") || (delay_str == "3")) {
        return EXPONENTIAL;
    } else if ((delay_str == "reverse_poisson") || (delay_str == "4")) {
        return REVERSE_POISSON;
    } else if ((delay_str == "reverse_exponential") || (delay_str == "5")) {
        return REVERSE_EXPONENTIAL;
    }
    return INVALID_DELAY;
}

void
PHOLDAgent::printDelayDistrib(std::ostream& os) {
    // Generate occurrences of random delay delay values.
    std::map<int, int> hist;
    for (int n = 0; (n < 10000); ++n) {
        ++hist[getDelay(delayType)];
    }
    // Determine highest frequency of occurrences to scale histogram.
    int maxFreq = 0;
    for (const auto& p : hist) {
        maxFreq = std::max(maxFreq, p.second);
    }
    // Now print vertical histogram (maxFreq == 75 stars).
    for (const auto& p : hist) {
        os << p.first << " ("  << p.second << ") "
           << std::string(p.second * 75 / maxFreq, '*') << std::endl;
    }
}

#endif
