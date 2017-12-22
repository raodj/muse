#ifndef MUSE_MULTI_BLOCKING_MT_QUEUE_CPP
#define MUSE_MULTI_BLOCKING_MT_QUEUE_CPP

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
#include "mpi-mt/MultiBlockingMTQueue.h"

// Switch to muse namespace to streamline code below
using namespace muse;

template <typename MutexType>
MultiBlockingMTQueue<MutexType>::MultiBlockingMTQueue(int numSubQueues,
                                                      int reserve)
    : subQueues(numSubQueues, SingleBlockingMTQueue<MutexType>(reserve)) {
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

template <typename MutexType>
void
MultiBlockingMTQueue<MutexType>::add(int srcThrIdx, int destThrIdx,
                                      muse::Event* event) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    ASSERT( event != NULL );
    // Add event to one of the sub-queues based on the sender agent's ID
    const muse::AgentID receiver = event->getReceiverAgentID();
    DEBUG(std::cout << (receiver & bitMask) << std::endl);
    subQueues[receiver & bitMask].add(srcThrIdx, destThrIdx, event);
}

template <typename MutexType>
void
MultiBlockingMTQueue<MutexType>::add(int srcThrIdx, int destThrIdx,
                                      EventContainer& eventList) {
    UNUSED_PARAM(srcThrIdx);
    UNUSED_PARAM(destThrIdx);
    
    if (eventList.empty()) {
        return;  // nothing to be done.
    }
    // Add all event to one of the sub-queues
    const muse::AgentID receiver = eventList.front()->getReceiverAgentID();
    subQueues[receiver & bitMask].add(srcThrIdx, destThrIdx, eventList);    
}

template <typename MutexType>
void
MultiBlockingMTQueue<MutexType>::removeAll(EventContainer& eventList,
                                           int destThrIdx, int maxEvents) {
    // Remove events from each sub-queue
    for (size_t i = 0; (i < subQueues.size()); i++) {
        // Add events to the eventList from the sub-queue
        subQueues[i].removeAll(eventList, destThrIdx, maxEvents);
    }
}

#endif
