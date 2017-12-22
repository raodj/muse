#ifndef MUSE_SINGLE_BLOCKING_MT_QUEUE_CPP
#define MUSE_SINGLE_BLOCKING_MT_QUEUE_CPP

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

#include "mpi-mt/SingleBlockingMTQueue.h"

// Switch to muse namespace to streamline code below
using namespace muse;

template <typename MutexType>
SingleBlockingMTQueue<MutexType>::SingleBlockingMTQueue(int reserve) {
    // Allocate some initial size.
    queue.reserve(reserve);
}

template <typename MutexType>
void
SingleBlockingMTQueue<MutexType>::add(int srcThrIdx, int destThrIdx,
                                      muse::Event* event) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    ASSERT( event != NULL );
    // Lock the mutex to get exclusive access to the actual queue
    std::lock_guard<MutexType> lock(queueMutex);
    queue.push_back(event);
}

template <typename MutexType>
void
SingleBlockingMTQueue<MutexType>::add(int srcThrIdx, int destThrIdx,
                                      EventContainer& eventList) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    
    if (eventList.empty()) {
        return;  // nothing to be done.
    }
    // Lock the mutex to get exclusive access to the actual queue
    std::lock_guard<MutexType> lock(queueMutex);
    // Add all the events.
    queue.insert(queue.end(), eventList.begin(), eventList.end());
    // Clear event list to show all events have been added.
    eventList.clear();
}

template <typename MutexType>
void
SingleBlockingMTQueue<MutexType>::removeAll(EventContainer& eventList,
                                            int destThrIdx, int maxEvents) {
    UNUSED_PARAM(destThrIdx);
    // Lock the mutex to get exclusive access to the actual queue
    std::lock_guard<MutexType> lock(queueMutex);
    // Now that we have exclusive access to the queue, finalize the
    // number of events to be dequeued.
    maxEvents = (maxEvents == -1 ? queue.size() :
                 std::min<int>(queue.size(), maxEvents));
    // Copy the desired number of events.
    eventList.insert(eventList.end(), queue.begin(), queue.begin() + maxEvents);
    // Remove the copied events from the queue.
    queue.erase(queue.begin(), queue.begin() + maxEvents);
}

#endif
