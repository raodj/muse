#ifndef TWO_TIER_HEAP_ADAPTER_CPP
#define TWO_TIER_HEAP_ADAPTER_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "TwoTierHeapAdapter.h"
#include "Agent.h"
#include "Utilities.h"

BEGIN_NAMESPACE(muse)

void
TwoTierHeapAdapter::getNextEvents(EventContainer& container) {
    ASSERT(container.empty());
    ASSERT(top() != NULL );
    
    const Time currTime = top()->getReceiveTime();  
    do {
        Event* event = top();
        // We should never process an anti-message.
        if (event->isAntiMessage()) { 
            std::cerr << "Anti-message Processing: " << *event << std::endl;
            std::cerr << "Trying to process an anti-message event, "
                      << "please notify MUSE developers of this issue"
                      << std::endl;
            abort();
        }
    
        // Ensure that the top event is greater than LVT
        if (event->getReceiveTime() <= agent->getTime(Agent::LVT)) {
            std::cerr << "Agent is being scheduled to process an event ("
                      << *event << ") that is at or below it LVT (LVT="
                      << agent->getTime(Agent::LVT) << ", GVT="
                      << agent->getTime(Agent::GVT)
                      << "). This is a serious error. Aborting.\n";
            std::cerr << *agent << std::endl;
            abort();
        }

        ASSERT(event->getReferenceCount() < 3);
        
        // We add the top event we popped to the event container
        event->increaseReference(); 
        container.push_back(event);

        DEBUG(std::cout << "Delivering: " << *event << std::endl);
        
        // Finally it is safe to remove this event from the eventPQ as
        // it has been added to the inputQueue
        pop();
    } while (!empty() && (TIME_EQUALS(top()->getReceiveTime(), currTime)));
}

END_NAMESPACE(muse)

#endif
