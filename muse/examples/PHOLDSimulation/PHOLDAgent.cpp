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
    
    delay = The delay time for the receive time.  This value is used
            as part of different random number
            generators/distributions to model different scenarios.    
*/

#include "PHOLDAgent.h"
#include "Event.h"
#include "MTRandom.h"
#include "Simulation.h"
#include "PHOLDEvent.h"
#include <cstdlib>
#include <cctype>
#include <random>
#include <algorithm>
#include <map>

using namespace std;
using namespace muse;

// The static maxDelay value
int PHOLDAgent::maxDelay = -1;

// The static maximum receiver range vaulue based on receiverDistType
int PHOLDAgent::maxRecvrRange = -1;

PHOLDAgent::PHOLDAgent(AgentID id, PholdState* state, int x, int y,
                       int n, int d, int lookAhead, double selfEvents,
                       size_t granularity, DelayType type,
                       int recvrRange, DelayType recvrDistType,
                       int extraEventSize) : 
    Agent(id, state), X(x),Y(y), N(n), delay(d), delayType(type),
    lookAhead(lookAhead),  selfEvents(selfEvents), rng(id),
    granularity(granularity), receiverRange(recvrRange),
    receiverDistType(recvrDistType), extraEventSize(extraEventSize) {
     // Setup the random seed used for generating self/other events
    seed = id;
    // Setup the maximum random delay value for reverse distributions
    if ((delay > 0) && (maxDelay == -1)) {
        maxDelay = getMaxDelayValue(delayType, delay);
        std::cout << "maxDelay = " << maxDelay << std::endl;
    }
    // Setup the maximum random receiver offset value for offset
    if ((receiverRange > 0) && (maxRecvrRange == -1)) {
        maxRecvrRange = getMaxDelayValue(receiverDistType, receiverRange);
        std::cout << "maxDelay = " << maxDelay << std::endl;
    }    
}

int
PHOLDAgent::getDelay(const DelayType delType, const int genParam) {
    switch (delType) {
    case UNIFORM: {
        std::uniform_int_distribution<int> uni(0, genParam);
        return uni(rng);
    }
    case POISSON: {
        std::poisson_distribution<int> poi(genParam);
        return poi(rng);
    }
    case EXPONENTIAL: {
        std::exponential_distribution<double> exp(genParam);
        return 2 * exp(rng);
    }
    case REVERSE_POISSON: {
        std::poisson_distribution<int> poi(genParam);
        return maxDelay - (poi(rng) % maxDelay);
    }
    case REVERSE_EXPONENTIAL: {
        std::exponential_distribution<double> exp(genParam);
        return maxDelay - ((int) (2 * exp(rng)) % maxDelay);
    }
    case INVALID_DELAY:
    default:
        return 0;
    }        
}

int
PHOLDAgent::getMaxDelayValue(DelayType delType, int delayVal) const {
    int maxDel = delayVal;  // the default delay value
    std::default_random_engine rnd;
    if ((delType == POISSON) || (delType == REVERSE_POISSON)) {
        maxDel = 0;
        std::poisson_distribution<int> poi(delayVal);
        for (int i = 0; (i < 100000); i++) {
            maxDel = std::max<int>(maxDel, poi(rnd));
        }
    }
    if ((delType == EXPONENTIAL) || (delType == REVERSE_EXPONENTIAL)) {
        maxDel = 0;
        std::exponential_distribution<double> exp(delayVal);
        for (int i = 0; (i < 100000); i++) {
            // The 2 factor is from http://en.cppreference.com/
            // example.  Since exponential distribution is real/double
            // the 2 factor provides a better spread.
            maxDel = std::max<int>(maxDel, 2 * exp(rnd));
        }        
    }
    return maxDel;
}
    
void
PHOLDAgent::initialize() throw (std::exception){
    // We generate N events with random receive times to self
    for (int i = 0; i < N; i++){
        const int RndDelay = (delay > 0) ? getDelay(delayType, delay) : 0;
        Time  receive(getTime() + 1 + RndDelay);
        ASSERT( receive > getTime() );
        if ( receive < Simulation::getSimulator()->getStopTime() ){
            const size_t eventSize = sizeof(PHOLDEvent) + extraEventSize;
            Event* e = Event::create<PHOLDEvent>(eventSize, getAgentID(),
                                                 receive, extraEventSize);
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

muse::AgentID
PHOLDAgent::getAdjacentAgentID() {
    PholdState *my_state = dynamic_cast<PholdState*>(getState());
    ASSERT( my_state != NULL );
    // Set default as current agent.
    muse::AgentID receiverAgentID = getAgentID();    
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
    // Handle wrap around cases in torroidal
    if(receiverAgentID < 0) {
        receiverAgentID += X * Y;
    }
    if(receiverAgentID >= (X * Y)) {
        receiverAgentID -= X * Y;
    }
    return receiverAgentID;
}

muse::AgentID
PHOLDAgent::getRecvrAgentID() {
    const int rndVal = getDelay(receiverDistType, receiverRange);
    // Set default as current agent.
    muse::AgentID receiverAgentID = getAgentID() + rndVal - (maxRecvrRange / 2);
    // Handle wrap around cases in torroidal
    while (receiverAgentID < 0) {
        receiverAgentID += X * Y;
    }
    while (receiverAgentID >= (X * Y)) {
        receiverAgentID -= X * Y;
    }
    return receiverAgentID;
}

void
PHOLDAgent::simEventProcessing(const muse::Event* event) const {
    ASSERT(dynamic_cast<const PHOLDEvent*>(event) != NULL);
    const PHOLDEvent* const evt = static_cast<const PHOLDEvent*>(event);
    ASSERT(evt->getEventSize() == int(sizeof(PHOLDEvent) + extraEventSize));    
    const char* const data = evt->getExtraData();
    // Do some basic data processing with events
    ASSERT(data != NULL);
    int sum = 0;
    for (int i = 0; (i < extraEventSize); i++) {
        sum += data[i];
    }
    // Ensure that the results are consistent.
    if (sum != extraEventSize) {
        std::cerr << "Extra bytes in events was not correct!\n";
    }
}

void
PHOLDAgent::executeTask(const EventContainer& events) {
    // For every event we get we send out one event
    for (size_t i = 0; (i < events.size()); i++) {
        // Simulate some event granularity
        simGranularity();
        // Simulate some operation on processing extra bytes on events
        if (extraEventSize > 0) {
            simEventProcessing(events[i]);
        }
        // First make a random receive time for the future
        const int RndDelay = (delay > 0) ? getDelay(delayType, delay) : 0;
        const Time  receiveTime(getTime() + lookAhead + RndDelay);

        if ( receiveTime < Simulation::getSimulator()->getStopTime() ){
            // Now we need to choose the agent to send this event to.
            muse::AgentID receiverAgentID = getAgentID();
            if (selfEvents < 0) {
                receiverAgentID = (receiverAgentID + i) % (X * Y);
            }
            if ((selfEvents >= 0) && (selfEvents < 1) &&
                (((rand_r(&seed) % 1000) / 1000.0) >= selfEvents)) {
                if (receiverRange <= 0) {
                    // Default operation -- randomly choose 1 of 4
                    // neighboring agents.
                    receiverAgentID = getAdjacentAgentID();
                } else {
                    // Use random distribution to determine receiver
                    // agent ID
                    receiverAgentID = getRecvrAgentID();
                }
            }
            ASSERT((receiverAgentID >= 0) && (receiverAgentID < X * Y));
            // Make event
            const size_t eventSize = sizeof(PHOLDEvent) + extraEventSize;
            Event* e = Event::create<PHOLDEvent>(eventSize, receiverAgentID,
                                                 receiveTime, extraEventSize);
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
        ++hist[getDelay(delayType, delay)];
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
