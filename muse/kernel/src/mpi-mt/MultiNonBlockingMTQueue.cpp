#ifndef MUSE_MULTI_NON_BLOCKING_MT_QUEUE_CPP
#define MUSE_MULTI_NON_BLOCKING_MT_QUEUE_CPP

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

#include <iostream>
#include <cmath>
#include "Event.h"
#include "mpi-mt/MultiNonBlockingMTQueue.h"

// Switch to muse namespace to streamline code below
using namespace muse;

MultiNonBlockingMTQueue::MultiNonBlockingMTQueue(int numSubQueues, int reserve,
                                                 int batchSize) :
    subQueues(numSubQueues), maxBatchSize(batchSize) {
    // Create sub-queues with fixed size
    for (int i = 0; (i < numSubQueues); i++) {
        subQueues[i] = new LockFreeQueue(reserve);
    }
    // The above subQueue constructor creates the specified numeber of
    // sub-queues, each with specified 'reserve' initial capacity.
    // Next setup bit-mask (with nearest power of 2) to select
    // sub-queue based on sending-thread-index value.
    const double pow2 = std::log2(numSubQueues);
    if (pow2 != (int) pow2) {
        std::cerr << "Warning: #sub-queues not an integral power of 2\n";
    }
    // Set bit bask to all 1 bits -- i.e. if pow2 is 3.17 (log2(9)) then
    // bit mask is set to 2^3 - 1 == 8 - 1 == 7 (or 0b111)
    bitMask = std::pow(2, (int) pow2) - 1;
    DEBUG(std::cerr << "bitMask set to: " << bitMask << std::endl);
}

MultiNonBlockingMTQueue::~MultiNonBlockingMTQueue() {
    for (LockFreeQueue* subQ : subQueues) {
        ASSERT(subQ->empty());
        delete subQ;
    }
}

void
MultiNonBlockingMTQueue::add(int srcThrIdx, int destThrIdx,
                             muse::Event* event) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    ASSERT( event != NULL );
    // Add event to one of the sub-queues based on the sender agent's ID
    const muse::AgentID receiver = event->getReceiverAgentID();
    DEBUG(std::cout << (receiver & bitMask) << std::endl);
    const int subQidx = receiver & bitMask;
    while (!subQueues[subQidx]->bounded_push(event)) {
        // Empty while loop that constantly retries to push an event.
        // This happens when the bonded sub-queue is full.  Ideally
        // looping here would be a rare case.
    }
}

void
MultiNonBlockingMTQueue::add(int srcThrIdx, int destThrIdx,
                             EventContainer& eventList) {
    if (eventList.empty()) {
        return;  // nothing to be done.
    }
    // Add all of the events. one at a time using overloaded method.
    for (Event* event : eventList) {
        add(srcThrIdx, destThrIdx, event);
    }
}

void
MultiNonBlockingMTQueue::removeAll(EventContainer& eventList,
                                   int destThrIdx, int maxEvents) {
    UNUSED_PARAM(destThrIdx);    
    // Remove events from each sub-queue
    for (LockFreeQueue* subQ : subQueues) {
        // We don't want to spin here consuming events as other
        // threads produce them.  We want to get a batch of events and
        // move on.  Since LockQueueQueue does not have a size()
        // method to tell number of elements in it, we will set an
        // arbitrary size of BATCH_SIZE
        int batchSize = maxBatchSize;
        if (maxEvents > 0) {
            // Incase there is a non-default maxEvents, honor that setting
            batchSize = std::min(maxEvents, maxBatchSize);
        }
        while (!subQ->empty() && (batchSize-- > 0)) {
            muse::Event* event;
            if (subQ->pop(event)) {
                // Store event in outgoing list
                eventList.push_back(event);
            } else {
                // No more events to pop. Don't retry. We will get the
                // events the next time around.
                break;
            }
        }
    }
}

#endif
