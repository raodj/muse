#ifndef TWO_TIER_HEAP_ADAPTER_H
#define TWO_TIER_HEAP_ADAPTER_H

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

#include "Utilities.h"
#include "BinaryHeapWrapper.h"

BEGIN_NAMESPACE(muse)

/** Extension to BinaryHeapWrapper to streamline operation in
    TwoTierHeap.

    This is a convenience class that was introduced as part of
    refactoring parts of event queue logic out of the Agent class.
    Most of the methods in this class were originally in Agent class.
    However, these methods deal with managing events which must be
    part of the event queue class hierarchy and not in Agent.
*/
class TwoTierHeapAdapter : public BinaryHeapWrapper {
    friend class TwoTierHeapEventQueue;
public:
    /** The constructor for this class.

        \note This class is designed to be created and used with the
        TwoTierHeapEventQueue.

        \see TwoTierHeapEventQueue::addAgent() method.
    */
    TwoTierHeapAdapter(muse::Agent* agent) : agent(agent) {}

    /** The destructor.

        The destructor does not have any specific task to perform.
     */
    ~TwoTierHeapAdapter() {}

protected:
    /** The getNextEvents method.
        
        This method is a helper that will grab the next
        set of events to be processed by this agent.
	
        \param[out] container The reference of the container into
        which events should be added.
    */
    void getNextEvents(EventContainer& container);

private:
    muse::Agent* const agent;
};

END_NAMESPACE(muse)

#endif
