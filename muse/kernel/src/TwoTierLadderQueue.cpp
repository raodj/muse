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

// The maximum number of buckets 1 rung can have.
#define MAX_BUCKETS 100

/** The number of sub-buckets to be used in each 2-tier bucket */
int muse::TwoTierBucket::t2k = 32;

// ----------------------[ TwoTierBucket methods ]-----------------------

muse::TwoTierBucket::~TwoTierBucket() {
    clear();
}

void
muse::TwoTierBucket::clear() {
    for (BktEventList& subBkt : subBuckets) {
        for (muse::Event* event : subBkt) {
            event->decreaseReference();
        }
    }
    count = 0;
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

void
muse::TwoTierBucket::push_back(BktEventList& dest, TwoTierBucket&& srcBkt) {
    for (BktEventList& subBkt : srcBkt.subBuckets) {
        dest.insert(dest.end(), subBkt.begin(), subBkt.end());
        subBkt.clear();
    }
    srcBkt.count = 0;
}

int
muse::TwoTierBucket::remove_after(muse::AgentID sender, const Time sendTime) {
    const size_t subBktIdx = hash(sender);
    int removedCount = remove_after(subBuckets[subBktIdx], sender, sendTime);
    count -= removedCount;  // Track remaining events
    return removedCount;
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
    return removedCount;
}

// This method is not performance critical as it is only called once
// at the end of simulation.
int
muse::TwoTierBucket::remove(muse::AgentID receiver) {
    size_t removedCount = 0;
    // Remove events from each sub-bucket.
    for (BktEventList& list : subBuckets) {
        // Use helper method to remove events.
        removedCount += remove(list, receiver);
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

// static helper method also used by TwoTierBottom.  This is called
// few times at the end of simulation.  So it is not performance
// critical.
int
muse::TwoTierBucket::remove(BktEventList& list, muse::AgentID receiver) {
    int removedCount = 0;  // statistics tracking
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
    return removedCount;  // let caller know the events removed
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

// -------------------------[ TwoTierBottom methods ]-------------------------

void
muse::TwoTierBottom::enqueue(muse::TwoTierBucket&& bucket) {
    // Move events from bucket into the bottom.
    TwoTierBucket::push_back(*this, std::move(bucket));
    // Now sort the whole bottom O(n*log(n)) operation
    std::sort(begin(), end(), TwoTierBottom::compare);
}

void
muse::TwoTierBottom::enqueue(muse::Event* event) {
    BktEventList::iterator iter =
        std::lower_bound(begin(), end(), event, compare);
    insert(iter, event);  // base class method.
}

void
muse::TwoTierBottom::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (empty()) {
        return;  // no events to provide
    }
    // Reference information used for checking in the loop below.
    const muse::Event*  nextEvt  = front();
    const muse::AgentID receiver = nextEvt->getReceiverAgentID();
    const muse::Time    currTime = nextEvt->getReceiveTime();
    // Move all events from bottom to the events-container for scheduling.
    do {
        // Front event is the lowest timestamp (or highest priority).
        muse::Event* event = front();
        events.push_back(event);
        pop_front();   // remove from bottom.
        // Check and work with the next event.
        nextEvt = (!empty() ? front() : NULL);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() && (nextEvt->getReceiverAgentID() == receiver) &&
             TIME_EQUALS(nextEvt->getReceiveTime(), currTime));
    DEBUG(validate());
}

// This method is used only for debugging. So it is not performance
// critical.
void
muse::TwoTierBottom::validate() const {
    if (empty()) {
        return;  // yes. bottom is valid.
    }
    // Ensure events are sorted in timestamp order.
    BktEventList::const_iterator next = cbegin();
    BktEventList::const_iterator prev = next++;
    while ((next != cend()) &&
           ((*next)->getReceiveTime() >= (*prev)->getReceiveTime())) {
        prev = next++;
    }
    if (next != cend()) {
        std::cout << "Error in LadderQueue.Bottom: Event " << **next
                  << " was found after " << **prev << std::endl;
    }
    ASSERT( next == cend() );
}

// -------------------------[ TwoTierRung methods ]-------------------------

muse::TwoTierRung::TwoTierRung(TwoTierBucket&& bkt, const Time minTS,
                               const double bktWidth) :
    rStartTS(minTS), rCurrTS(minTS), bucketWidth(bktWidth), currBucket(0),
    rungEventCount(0) {
    // Initialize variable to track maximum bucket count
    LQ_STATS(maxBkts = 0);
    // Ensure bucket width is not ridiculously small
    bucketWidth = std::max(MIN_BUCKET_WIDTH, bucketWidth);
    DEBUG(std::cout << "bucketWidth = " << bucketWidth << std::endl);
    ASSERT(bucketWidth > 0);
    ASSERT(rungEventCount == 0);
    // Move events from given bucket into buckets in this Rung.
    DEBUG(std::cout << "Adding " << bkt.size() << " events to rung\n");
    for (BktEventList& list : bkt.getSubBuckets()) {
        // Add all events from sub-buckets to various buckets in this rung.
        while (!list.empty()) {
            // Remove event from the top linked list.
            muse::Event* event = list.front();
            list.pop_front();
            // Add to the appropriate bucket in this rung using a
            // helper method in this class.
            enqueue(event);
        }
    }
    DEBUG(validateEventCounts());
}

void
muse::TwoTierRung::enqueue(muse::Event* event) {
    ASSERT(event != NULL);
    ASSERT(event->getReceiveTime() >= getCurrTime());
    // Compute bucket for this event based on equation #2 in LQ paper.
    size_t bucketNum = (event->getReceiveTime() - rStartTS) / bucketWidth;
    ASSERT(bucketNum >= currBucket);
    if (bucketNum >= bucketList.size()) {
        // Ensure bucket list of sufficient size
        bucketList.resize(bucketNum + 1);
        // update variable to track maximum bucket count
        LQ_STATS(maxBkts = std::max(maxBkts, bucketList.size()));
    }
    ASSERT(bucketNum < bucketList.size());
    // Add event into appropriate bucket
    bucketList[bucketNum].push_back(event);
    // Track number of events added to this Rung
    rungEventCount++;
}

muse::TwoTierBucket&&
muse::TwoTierRung::removeNextBucket(muse::Time& bktTime) {
    ASSERT(!empty());
    ASSERT(currBucket < bucketList.size());
    // Find next non-empty bucket in this rung (there has to be one as
    // the previous asserts passed necessary checks)
    while ((currBucket < bucketList.size()) && bucketList[currBucket].empty()) {
        currBucket++;
    }
    DEBUG(validateEventCounts());
    ASSERT(currBucket < bucketList.size());
    ASSERT(!bucketList[currBucket].empty());
    // Track events that will be removed when this method returns
    rungEventCount -= bucketList[currBucket].size();
    ASSERT(rungEventCount >= 0);
    // Save information about the bucket to be removed & returned.
    const int retBkt = currBucket;
    bktTime = rStartTS + (retBkt * bucketWidth);
    // Advance current bucket to next time.    
    currBucket++;
    rCurrTS = rStartTS + (currBucket * bucketWidth);
    // Sanity check on counters...
    if (currBucket >= bucketList.size()) {
        ASSERT(rungEventCount == 0);
    }
    return std::move(bucketList[retBkt]);
}

int
muse::TwoTierRung::remove_after(muse::AgentID sender, const Time sendTime
                                LQ_STATS(COMMA Avg& ceScanRung)) {
    if (empty() || (sendTime > getMaxRungTime())) {
        return 0;  // no events removed.
    }
    // Check each bucket in this rung and cancel out events.
    int numRemoved = 0;
    for (size_t bucketNum = currBucket; (bucketNum < bucketList.size());
         bucketNum++) {
        if (!bucketList[bucketNum].empty() &&
            (rStartTS + (bucketNum + 1) * bucketWidth) >= sendTime) {
            // This stats here is a bit bogus. We need to pass this
            // stats object down to the bucket to have it correctly updated.
            LQ_STATS(ceScanRung += bucketList[bucketNum].size());
            // Have the bucket remove necessary event(s)
            numRemoved += bucketList[bucketNum].remove_after(sender, sendTime);
        }
    }
    // Update events left in this rung.
    rungEventCount -= numRemoved;
    DEBUG(validateEventCounts());
    return numRemoved;
}

int
muse::TwoTierRung::remove(muse::AgentID receiver
                          LQ_STATS(COMMA Avg& ceScanRung)) {
    if (empty()) {
        return 0;  // no events to be removed.
    }
    // Have each bucket in the rung remove events
    int numRemoved = 0;
    for (size_t bucketNum = currBucket; (bucketNum < bucketList.size());
         bucketNum++) {
        if (!bucketList[bucketNum].empty()) {
            // This stat needs to be tracked by the bucket and not here.
            LQ_STATS(ceScanRung += bucketList[bucketNum].size());
            // Remove appropriate set of events.
            numRemoved += bucketList[bucketNum].remove(receiver);
        }
    }
    rungEventCount -= numRemoved;
    DEBUG(validateEventCounts());
    return numRemoved;
}

void
muse::TwoTierRung::validateEventCounts() const {
    int numEvents = 0;
    for (const auto& bucket : bucketList)  {
        numEvents += bucket.size();
    }
    if (numEvents != rungEventCount) {
        DEBUG(std::cout << "Rung event count mismatch! Expecting: "
                        << rungEventCount << " events, but found: "
                        << numEvents << "." << std::endl);
        ASSERT(numEvents == rungEventCount);
    }
}

// Method called just before a rung is removed from the ladder queue.
void
muse::TwoTierRung::updateStats(Avg& avgBktCnt) const {
    LQ_STATS(avgBktCnt += maxBkts);
}

// This method is used only for troubleshooting/debugging purposes.
bool
muse::TwoTierRung::haveBefore(const Time recvTime) const {
    for (size_t i = 0; (i < bucketList.size()); i++) {
        if (bucketList[i].haveBefore(recvTime)) {
            return true;
        }
    }
    return false;
}

#endif
