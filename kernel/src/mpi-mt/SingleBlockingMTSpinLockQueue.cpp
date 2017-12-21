#ifndef MUSE_SINGLE_BLOCKING_MT_SPIN_LOCK_QUEUE_CPP
#define MUSE_SINGLE_BLOCKING_MT_SPIN_LOCK_QUEUE_CPP

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include "mpi-mt/SingleBlockingMTSpinLockQueue.h"

// Switch to muse namespace to streamline code below
using namespace muse;

SingleBlockingMTSpinLockQueue::SingleBlockingMTSpinLockQueue(int reserve) {
    // Allocate some initial size.
    queue.reserve(reserve);
}

void
SingleBlockingMTSpinLockQueue::add(int srcThrIdx, int destThrIdx,
                           muse::Event* event) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    ASSERT( event != NULL );
    // Lock the spin lock to get exclusive access to the actual queue
    queueSL.lock();
    queue.push_back(event);
    // Unlock the spin lock!
    queueSL.unlock();
}

void
SingleBlockingMTSpinLockQueue::add(int srcThrIdx, int destThrIdx,
                                   EventContainer& eventList) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    
    if (eventList.empty()) {
        return;  // nothing to be done.
    }
    // Lock the spin lock to get exclusive access to the actual queue
    queueSL.lock();
    // Add all the events.
    queue.insert(queue.end(), eventList.begin(), eventList.end());
    // Clear event list to show all events have been added.
    eventList.clear();
    // Don't forget to unlock the spin lock
    queueSL.unlock();
}

EventContainer
SingleBlockingMTSpinLockQueue::removeAll(int destThrIdx, int maxEvents) {
    UNUSED_PARAM(destThrIdx);
    // Lock the sping lock to get exclusive access to the actual queue
    queueSL.lock();
    // Now that we have exclusive access to the queue, finalize the
    // number of events to be dequeued.
    maxEvents = (maxEvents == -1 ? queue.size() :
                 std::min<int>(queue.size(), maxEvents));
    // Copy the desired number of events.
    EventContainer retVal(queue.begin(), queue.begin() + maxEvents);
    // Remove the copied events from the queue.
    queue.erase(queue.begin(), queue.begin() + maxEvents);
    // Don't forget to unlock the spin lock
    queueSL.unlock();
    // Return the local list back to the caller.
    return retVal;
}

#endif
