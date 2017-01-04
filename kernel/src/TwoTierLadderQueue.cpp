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
    ASSERT(srcBkt.subBuckets.size() == (size_t) t2k);
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
    clear();
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
    LQ2T_STATS(maxBkts = 0);
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

bool
muse::TwoTierRung::canContain(muse::Event* event) const {
    const muse::Time recvTime = event->getReceiveTime();
    const int bucketNum = (recvTime - rStartTS) / bucketWidth;
    return ((bucketNum >= (int) currBucket) && (recvTime >= rStartTS));
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
        LQ2T_STATS(maxBkts = std::max(maxBkts, bucketList.size()));
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
                                LQ2T_STATS(COMMA Avg& ceScanRung)) {
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
            LQ2T_STATS(ceScanRung += bucketList[bucketNum].size());
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
                          LQ2T_STATS(COMMA Avg& ceScanRung)) {
    if (empty()) {
        return 0;  // no events to be removed.
    }
    // Have each bucket in the rung remove events
    int numRemoved = 0;
    for (size_t bucketNum = currBucket; (bucketNum < bucketList.size());
         bucketNum++) {
        if (!bucketList[bucketNum].empty()) {
            // This stat needs to be tracked by the bucket and not here.
            LQ2T_STATS(ceScanRung += bucketList[bucketNum].size());
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
    LQ2T_STATS(avgBktCnt += maxBkts);
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

void
muse::TwoTierRung::prettyPrint(std::ostream& os) const {
    // Compute minimum, maximum, empty, and average bucket sizes.
    size_t minBkt = -1U, maxBkt = 0, emptyBkt = 0, sizeSum = 0;
    for (const TwoTierBucket& bkt : bucketList) {
        if (!bkt.empty()) {
            minBkt   = std::min(minBkt, bkt.size());
            maxBkt   = std::max(maxBkt, bkt.size());
            sizeSum += bkt.size();
        } else {
            emptyBkt++;
        }
    }
    double avgBktSz = sizeSum / (double) (bucketList.size() - emptyBkt);
    os << "start time="   << rStartTS    << ", curr time=" << rCurrTS
       << ", bkt. width=" << bucketWidth << ", bkt count=" << bucketList.size()
       << ", curr buckt=" << currBucket  << ", events="    << rungEventCount
       << ", min bkt="    << minBkt      << ", maxBkt="    << maxBkt
       << ", empty bkt="  << emptyBkt    << ", avg size="  << avgBktSz
       << std::endl;
}

// --------------------[ TwoTierLadderQueue methods ]--------------------

void
muse::TwoTierLadderQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);    
    LQ2T_STATS({
            // Collect final bucket counts from the ladder
            for (size_t i = 0; (i < ladder.size()); i++) {
                ladder[i].updateStats(avgBktCnt);
            }
            // Compute net number of compares for ladderQ
            // const long comps = log2(botLen.getMean()) * botLen.getSum();
            // std::make_heap has 3N time complexity.
            const long comps = 3 * botLen.getSum() +
                log2(botLen.getMean()) * botLen.getSum() / 3;
            os << "Events cancelled from top   : "   << ceTop
               << "\nEvents scanned in top       : " << ceScanTop
               << "\nEvents cancelled from ladder: " << ceLadder
               << "\nEvents scanned from ladder  : " << ceScanLadder
               << "\nEvents cancelled from bottom: " << ceBot
               << "\nEvents scanned from bottom  : " << ceScanBot
               << "\nInserts into top            : " << insTop
               << "\nInserts into rungs          : " << insLadder
               << "\nInserts into bottom         : " << insBot
               << "\nMax rung count              : " << maxRungs
               << "\nAverage #buckets per rung   : " << avgBktCnt
               << "\nAverage bottom size         : " << botLen
               << "\nAverage bucket width        : " << avgBktWidth
               << "\nCompare estimate            : " << comps
               << std::endl;
    });
}

void
muse::TwoTierLadderQueue::enqueue(muse::Event* event) {
    if (top.getStartTime() < event->getReceiveTime()) {
        DEBUG(std::cout << "Added to top: " << *event << std::endl);
        top.add(event);
        LQ2T_STATS(insTop++);
        return;
    }
    // Try to see if the event fits in the ladder
    size_t rung = 0;
    while ((rung < ladder.size()) && !ladder[rung].canContain(event)) {
        ASSERT((rung == 0) || ladder[rung].empty() ||
               (ladder[rung - 1].getCurrTime() >= ladder[rung].getCurrTime()));
        rung++;
    }
    if (rung < ladder.size()) {
        DEBUG(ASSERT(bottom.empty() ||
                     (event->getReceiveTime() > bottom.maxTime())));
        ladder[rung].enqueue(event);
        ladderEventCount++;  // Track events added to the ladder
        DEBUG(std::cout << "Added to rung " << rung  << "(max bottom: "
              << bottom.maxTime() << "): " << *event << "\n");
        LQ2T_STATS(insLadder++);
        return;
    }
    // Event does not fit in the ladder. Note that 2-tier ladder queue
    // we do not move events out of bottom back into the ladder to
    // ensure short bottom. Instead, we allow bottom to grow
    // long. Consequently, event must go into bottom At this point
    // bottom must be able to contain the event, so enqueue it.
    bottom.enqueue(event);
    DEBUG(ASSERT(!haveBefore(bottom.front()->getReceiveTime())));
    DEBUG(std::cout << "Added to bottom: " << *event << std::endl);
    LQ2T_STATS(insBot++);
}

// Implementation close to the version from the paper.
muse::TwoTierBucket&&
muse::TwoTierLadderQueue::recurseRung() {
    ASSERT(!empty());
    // First, find the next non-empty bucket across multiple rungs in
    // the ladder
    LQ2T_STATS(bool isLastRung = true);
    while (!ladder.empty() && ladder.back().empty()) {
        // Track statistics if enabled.
        LQ2T_STATS({
                if (!isLastRung) {
                    ladder.back().updateStats(avgBktCnt);
                }
            });
        // Remove empty rung (i.e., NRung--) at the end.
        ladder.pop_back();
        // In the next iteration the rung to remove (if any) was not
        // the last rung in the ladder
        LQ2T_STATS(isLastRung = false);
    }
    ASSERT(!ladder.empty());
    // Now the last rung in ladder is the rung that has the next
    // bucket of events.
    muse::Time bktTime    = 0;
    TwoTierRung& lastRung = ladder.back();    
    TwoTierBucket&& bkt   = lastRung.removeNextBucket(bktTime);
    ASSERT(!ladder.empty());
    // Check and create new rung in the ladder if the bucket is large.
    if ((bkt.size() > LQ2T_THRESH) && (ladder.size() < MAX_RUNGS) &&
        (lastRung.getBucketWidth() > MIN_BUCKET_WIDTH)) {
        // Note: Here bucket width can dip below MIN_BUCKET_WIDTH. But
        // that is needed to ensure consistent ladder setup.
        const double bucketWidth = (lastRung.getBucketWidth() + bkt.size() -
                                    1.0) / bkt.size();
        ladder.push_back(TwoTierRung(std::move(bkt), bktTime, bucketWidth));
        DEBUG(std::cout << "2. Bucket width: " << bucketWidth << std::endl);
        LQ2T_STATS(avgBktWidth += bucketWidth);
        LQ2T_STATS(maxRungs = std::max(maxRungs, ladder.size()));
        return recurseRung();  // Recurse now looking at newly added rung
    }
    // Track events being removed from the ladder
    ladderEventCount -= bkt.size();
    ASSERT(ladderEventCount >= 0);
    // Return bucket being removed.
    return std::move(bkt);
}

// Move events from ladder (or top) into bottom.
void
muse::TwoTierLadderQueue::populateBottom() {
    if (!bottom.empty()) {
        return;
    }
    if (ladderEventCount == 0) {   // NRung == 0
        if (top.empty()) {
            // There are no events in the ladder queue in this case
            ASSERT(empty());
            return;
        }
        // Move all events from top into buckets in first rung of the ladder!
        ladder.clear();
        ladderEventCount += top.size();     // Track events in ladder
        ladder.push_back(TwoTierRung(std::move(top))); // Move events to ladder
        LQ2T_STATS(maxRungs = std::max(maxRungs, ladder.size()));
        LQ2T_STATS(avgBktWidth += ladder.back().getBucketWidth());
        DEBUG(std::cout << "3. Bucket width: "
                        << ladder.back().getBucketWidth() << std::endl);
        DEBUG(prettyPrint(std::cout));
        ASSERT(top.empty());
    }
    // Bottom is empty. So we need to move events from the current
    // bucket in the ladder to bottom.
    ASSERT(!ladder.empty());
    ASSERT(bottom.empty());
    bottom.enqueue(recurseRung());  // Transfer bucket_k into bottom
    ASSERT(!bottom.empty());
    DEBUG(ASSERT(!haveBefore(bottom.front()->getReceiveTime())));
    LQ2T_STATS(botLen += bottom.size());
}

int
muse::TwoTierLadderQueue::remove_after(muse::AgentID sender,
                                       const Time sendTime) {
    LQ2T_STATS(ceScanTop += top.size());    
    int numRemoved = top.remove_after(sender, sendTime);
    LQ2T_STATS(ceTop += numRemoved);
    // Cancel out events in each rung of the ladder.
    for (auto& rung : ladder) {
        const int rungEvtRemoved =
            rung.remove_after(sender, sendTime LQ2T_STATS(COMMA ceScanLadder));
        ladderEventCount  -= rungEvtRemoved;
        numRemoved        += rungEvtRemoved;
        LQ2T_STATS(ceLadder += rungEvtRemoved);
    }
    LQ2T_STATS(ceScanBot += bottom.size());
    const int botRemoved  = bottom.remove_after(sender, sendTime);
    numRemoved           += botRemoved;
    LQ2T_STATS(ceBot     += botRemoved);
    return numRemoved;
}

bool
muse::TwoTierLadderQueue::haveBefore(const Time recvTime,
                                     const bool checkBottom) const {
    if (top.haveBefore(recvTime)) {
        std::cout << "Top has event that is <= " << recvTime << std::endl;
        prettyPrint(std::cout);
        return true;
    }
    for (size_t rung = 0; (rung < ladder.size()); rung++) {
        if (ladder[rung].haveBefore(recvTime)) {
            std::cout << "Rung #" << rung << " has event that is <= "
                      << recvTime << std::endl;
            prettyPrint(std::cout);
            return true;
        }
    }
    
    if (checkBottom && bottom.haveBefore(recvTime)) {
        std::cout << "Bottom has event that is <= " << recvTime << std::endl;
        prettyPrint(std::cout);
        return true;
    }
    return false;
}

// ------------------------[  EventQueue implementation ]----------------------

void*
muse::TwoTierLadderQueue::addAgent(muse::Agent* agent) {
    UNUSED_PARAM(agent);
    return NULL;
}

void
muse::TwoTierLadderQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    const AgentID receiver = agent->getAgentID();
    // Remove events for agent from top
    LQ2T_STATS(ceScanTop += top.size());
    int numRemoved    = top.remove(receiver);
    LQ2T_STATS(ceTop += numRemoved);
    
    // Next remove events for agent from all the rungs in the ladder
    for (TwoTierRung& rung : ladder) {
        int rungEvtRemoved = rung.remove(agent->getAgentID()
                                         LQ2T_STATS(COMMA ceBot));
        ladderEventCount  -= rungEvtRemoved;
        numRemoved        += rungEvtRemoved;
        LQ2T_STATS(ceLadder += rungEvtRemoved);
    }
    // Finally remove events from bottom for the agent.
    LQ2T_STATS(ceScanBot += bottom.size());
    const int botRemoved  = bottom.remove(receiver);
    numRemoved           += botRemoved;
    LQ2T_STATS(ceBot     += botRemoved);
}


muse::Event*
muse::TwoTierLadderQueue::front() {
    if (empty()) {
        // Nothing to return.
        return NULL;
    }
    if (bottom.empty()) {
        populateBottom();
        DEBUG(prettyPrint(std::cout));
    }
    ASSERT(!bottom.empty());
    return bottom.front();
}

void
muse::TwoTierLadderQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (empty()) {
        // No events to dequeue.
        return;
    }
    // We only dequeue from bottom. So ensure it has events in it.
    if (bottom.empty()) {
        // Move events from top or a ladder rung into bottom.
        populateBottom();
    }
    ASSERT(!bottom.empty());
    bottom.dequeueNextAgentEvents(events);
    ASSERT(!events.empty());
    DEBUG(ASSERT(!haveBefore(events.front()->getReceiveTime())));
}

// The main interface method used by MUSE to schedule event.
void
muse::TwoTierLadderQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    UNUSED_PARAM(agent);
    event->increaseReference();
    enqueue(event);
}

// Method for block addition (typically used during rollback recovery)
void
muse::TwoTierLadderQueue::enqueue(muse::Agent* agent,
                                  muse::EventContainer& events) {
    UNUSED_PARAM(agent);
    for (auto& curr : events) {
        enqueue(curr);
    }
    events.clear();
}

// Method to cancel all events in the 2-tier heap.
int
muse::TwoTierLadderQueue::eraseAfter(muse::Agent* dest,
                                     const muse::AgentID sender,
                                     const muse::Time sentTime) {
    UNUSED_PARAM(dest);
    return remove_after(sender, sentTime);
}

void
muse::TwoTierLadderQueue::prettyPrint(std::ostream& os) const {
    // Print information on top.
    os << "Top: Events=" << top.size()
       << ", startTime=" << top.getStartTime()
       << ", minTime="   << top.getMinTime()
       << ", maxTime="   << top.getMaxTime()  << std::endl;
    // Print info on each rung of the ladder
    std::cout << "Ladder (rungs=" << ladder.size() << "):\n";
    for (size_t i = 0; (i < ladder.size()); i++) {
        os << "[" << i << "]: ";
        ladder[i].prettyPrint(os);
    }
    // Print info on bottom
    os << "Bottom: Events=" << bottom.size()
       << ", min="          << (!bottom.empty() ? bottom.findMinTime() : -1.0)
       << ", max="          << (!bottom.empty() ? bottom.maxTime()     : -1.0)
       << std::endl;
}

#endif
