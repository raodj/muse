#ifndef TWO_TIER_LADDER_QUEUE_CPP
#define TWO_TIER_LADDER_QUEUE_CPP

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

#include <algorithm>
#include <functional>
#include "TwoTierLadderQueue.h"

#define MAX_BUCKETS 100
#define MIN_BUCKET_WIDTH 1.0

/** The number of sub-buckets to be used in each 2-tier bucket */
int muse::TwoTierBucket::t2k = 32;

// ----------------------[ TwoTierBucket methods ]-----------------------

muse::TwoTierBucket::~TwoTierBucket() {
    for (BktEventList& subBkt : subBuckets) {
        for (muse::Event* event : subBkt) {
            event->decreaseReference();
        }
    }
}

void
muse::TwoTierBucket::push_back(TwoTierBucket&& srcBkt) {
    ASSERT(srcBkt.subBuckets.size() == t2k);
    ASSERT(srcBkt.subBuckets.size() == subBuckets.size());
    // Move evens from srcBkt into corresponding sub-buckets
    for (int idx = 0; (idx < t2k); idx++) {
        // Obtain reference to subbucket to be moved.
        BktEventList& src  = srcBkt.subBuckets[idx];
        BktEventList& dest = subBuckets[idx];
        // Move events from src to dest.
        dest.insert(dest.end(), src.begin(), src.end());
        // Update counters (also used to troubleshooting).
        count += src.size();
        // Clear out the source as events have been logically moved
        // out of it.
        src.clear();
    }
}

// Helper method to remove events from a sub-bucket.
int
muse::TwoTierBucket::remove_after(BktEventList& list, muse::AgentID sender,
                                  const Time sendTime) {
    size_t removedCount = 0;
    size_t curr = 0;
    while (curr < list.size()) {
        muse::Event* const event = list[curr];
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sendTime)) {
            // Free-up event.
            event->decreaseReference();
            removedCount++;
            // To minimize removal time replace entry with last one
            // and pop the last entry off.
            list[curr] = list.back();
            list.pop_back();
        } else {
            curr++;
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

// This method is not performance critical as it is only called once
// at the end of simulation.
int
muse::TwoTierBucket::remove(muse::AgentID receiver) {
    size_t removedCount = 0;
    // Remove events from each sub-bucket.
    for (BktEventList& list : subBuckets) {
        // Linear scan through events in a given sub-bucket
        BktEventList::iterator curr = list.begin();
        while (curr != list.end()) {
            if ((*curr)->getReceiverAgentID() == receiver) {
                (*curr)->decreaseReference();
                curr = list.erase(curr);                
                removedCount++;
            } else {
                curr++;
            }
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

// This method is not performance critical.  It is used only for
// troubleshooting/debugging
bool
muse::TwoTierBucket::haveBefore(const Time recvTime) const {
    for (const BktEventList& list : subBuckets) {
        for (const muse::Event* const event : list) {
            if (event->getReceiveTime() <= recvTime) {
                return true;
            }
        }
    }
    return false;
}

// Actually counts events in each bucket for validation purposes.
// This method is not performance critical.  It is used only for
// troubleshooting/debugging
size_t
muse::TwoTierBucket::getEventCount() const {
    int sum = 0;
    for (const BktEventList& subBkt : subBuckets) {
        sum += subBkt.size();
    }
    return sum;  // total number of events
}

// -------------------------[ TwoTierTop methods ]-------------------------

// Helper method called from constructor and when events are moved
// from top into ladder.
void
muse::TwoTierTop::reset(const Time startTime) {
    minTS    = TIME_INFINITY;
    maxTS    = 0;
    topStart = startTime;
}

void
muse::TwoTierTop::add(muse::Event* event) {
    push_back(event);   // Call base-class method.
    // Update running timestamps.
    minTS = std::min(minTS, event->getReceiveTime());
    maxTS = std::max(maxTS, event->getReceiveTime());
}


#endif
