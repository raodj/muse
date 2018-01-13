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
#define MAX_BUCKETS 50

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
            TwoTierLadderQueue::decreaseReference(event);
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
    // Move all entries from each sub-bucket in srcBkt to the end of dest.
    for (BktEventList& subBkt : srcBkt.subBuckets) {
        dest.insert(dest.end(), subBkt.begin(), subBkt.end());
        subBkt.clear();
    }
    // Reset count as part of move semantics
    srcBkt.count = 0;
}

int
muse::TwoTierBucket::remove_after(muse::AgentID sender, const Time sendTime
                                  LQ2T_STATS(COMMA Avg& scans)) {
    const size_t subBktIdx = hash(sender);
    LQ2T_STATS(scans += subBuckets[subBktIdx].size());
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
            TwoTierLadderQueue::decreaseReference(event);
            removedCount++;
            // To minimize removal time replace entry with last one
            // and pop the last entry off.
            list[curr] = list.back();
            list.pop_back();
        } else {
            curr++;  // on to the next event in the list
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

// static helper method also used by OneTierBottom.  This is called
// few times at the end of simulation.  So it is not performance
// critical.
int
muse::TwoTierBucket::remove(BktEventList& list, muse::AgentID receiver) {
    int removedCount = 0;  // statistics tracking
    // Linear scan through events in a given sub-bucket
    BktEventList::iterator curr = list.begin();
    while (curr != list.end()) {
        if ((*curr)->getReceiverAgentID() == receiver) {
            TwoTierLadderQueue::decreaseReference(*curr);
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
    push_back<SenderID>(event);   // Call base-class method.
    // Update running timestamps.
    minTS = std::min(minTS, event->getReceiveTime());
    maxTS = std::max(maxTS, event->getReceiveTime());
}

double
muse::TwoTierTop::getBucketWidth() const {
    DEBUG(std::cout << "minTS=" << minTS << ", maxTS=" << maxTS
          << ", size=" << size() << std::endl);
    // Compute preferred bucket width as per Tang et al paper
    double bktWidth = std::max((maxTS - minTS + size() - 1.0) / size(), 0.01);
    // Don't let bucket count exceed max number of buckets
    const int lastBkt = (maxTS - minTS) / bktWidth;
    if (lastBkt > MAX_BUCKETS) {
        // The number of buckets would exceed threshold. So rework
        // bucket size such that it will not eceed MAX_BUCKETS.
        bktWidth = std::max((maxTS - minTS) / MAX_BUCKETS, 0.01);
    }
    return bktWidth;
}


// -------------------------[ OneTierBottom methods ]-------------------------

void
muse::OneTierBottom::enqueue(muse::TwoTierBucket&& bucket) {
    // Move events from bucket into the bottom.
    TwoTierBucket::push_back(*this, std::move(bucket));
    // Now sort the whole bottom O(n*log(n)) operation
    std::sort(begin(), end(), OneTierBottom::compare);
    DEBUG(validate());
}

void
muse::OneTierBottom::enqueue(muse::Event* event) {
    BktEventList::iterator iter =
        std::lower_bound(begin(), end(), event, compare);
    insert(iter, event);  // base class method.
    DEBUG(validate());
}

void
muse::OneTierBottom::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (empty()) {
        return;  // no events to provide
    }
    // Reference information used for checking in the loop below.
    const muse::Event*  nextEvt  = back();
    const muse::AgentID receiver = nextEvt->getReceiverAgentID();
    const muse::Time    currTime = nextEvt->getReceiveTime();
    // Move all events from bottom to the events-container for scheduling.
    do {
        // Back event is the lowest timestamp (or highest priority)
        // based on sorting order in OneTierBottom::compare()
        muse::Event* event = back();
        events.push_back(event);
        pop_back();   // remove from bottom.
        // erase(begin());
        // Check and work with the next event.
        nextEvt = (!empty() ? back() : NULL);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() && (nextEvt->getReceiverAgentID() == receiver) &&
             TIME_EQUALS(nextEvt->getReceiveTime(), currTime));
    DEBUG(validate());
}

double
muse::OneTierBottom::getBucketWidth() const {
    if (empty()) {
        return 0;
    }
    ASSERT(front() != NULL);
    ASSERT(back()  != NULL);
    // Assumes that bottom is sorted with the lowest timestamp at the
    // end for fast pop_back
    const double maxTS = front()->getReceiveTime();
    const double minTS = back()->getReceiveTime();
    ASSERT(maxTS >= minTS);
    ASSERT(size() > 0);
    double bktWidth = (maxTS - minTS + size() - 1.0) / size();
    // Don't let bucket width get so small that it will exceed the
    // max_bucket threshold to handle the events.
    const int lastBkt = (maxTS - minTS) / bktWidth;
    if (lastBkt > MAX_BUCKETS) {
        // The number of buckets would exceed threshold. So rework
        // bucket size such that it will not eceed MAX_BUCKETS.
        bktWidth = std::max((maxTS - minTS) / MAX_BUCKETS, 0.01);
    }
    return bktWidth;
}

int
muse::OneTierBottom::remove_after(muse::AgentID sender, const Time sendTime) {
    // Since bucket is sorted we can shortcircuit scan if last event's
    // time is less-or-equal to sendTime.
    if (empty() || (sendTime >= front()->getReceiveTime())) {
        return -1;  // Since bucket does not have events to be cancelled.
    }
    size_t removedCount = 0;
    iterator curr = begin();
    while (curr != end()) {
        muse::Event* const event = *curr;
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sendTime)) {
            // Free-up event.
            TwoTierLadderQueue::decreaseReference(event);
            removedCount++;
            // In sorted mode we have to preserve the order. So
            // cannot swap & pop in this situation
            curr = erase(curr);
        } else {
            curr++;  // onto next event
        }
    }
    return removedCount;
}

// This method is used only for debugging. So it is not performance
// critical.
void
muse::OneTierBottom::validate() const {
    if (empty()) {
        return;  // yes. bottom is valid.
    }
    // Ensure events are sorted in timestamp order.
    BktEventList::const_reverse_iterator next = crbegin();
    BktEventList::const_reverse_iterator prev = next++;
    while ((next != crend()) &&
           ((*next)->getReceiveTime() >= (*prev)->getReceiveTime())) {
        prev = next++;
    }
    if (next != crend()) {
        std::cout << "Error in LadderQueue.Bottom: Event " << **next
                  << " was found after " << **prev << std::endl;
    }
    ASSERT( next == crend() );
}

// -------------------------[ TwoTierRung methods ]-------------------------

muse::TwoTierRung&
muse::TwoTierRung::operator=(TwoTierRung&& src) {
    rStartTS       = std::move(src.rStartTS);
    rCurrTS        = std::move(src.rCurrTS);
    bucketWidth    = std::move(src.bucketWidth);
    currBucket     = std::move(src.currBucket);
    bucketList     = std::move(src.bucketList);
    rungEventCount = std::move(src.rungEventCount);
    // Move over all the statistics information
    LQ2T_STATS(maxBkts    = std::move(src.maxBkts));
    LQ2T_STATS(numRedistr = std::move(src.numRedistr));
    return *this;
}

muse::TwoTierRung::TwoTierRung(TwoTierRung&& src, const double newBktWidth) :
    rStartTS(src.rStartTS), bucketWidth(newBktWidth), rungEventCount(0) {
    // Track the maximum number of buckets from source
    LQ2T_STATS(maxBkts = std::move(src.maxBkts));
    // The number of times events were re-bucketed
    LQ2T_STATS(numRedistr = std::move(src.numRedistr));
    // Set the current bucket for this rung based on new bucket width.
    currBucket = (src.currBucket * src.bucketWidth) / bucketWidth;
    ASSERT(currBucket <= src.currBucket);
    // Setup the current timestamp value based on curr bucket
    // rCurrTS = rStartTS + (currBucket * bucketWidth);
    rCurrTS = src.rCurrTS;
    // Redistribute events from source rung to this rung.
    for (size_t bkt = 0; (bkt < src.bucketList.size()); bkt++) {
        move(std::move(src.bucketList[bkt]));
    }
    ASSERT(rungEventCount == src.rungEventCount);
    src.rungEventCount = 0;
}
    
void
muse::TwoTierRung::move(TwoTierBucket&& bkt, const Time minTS,
                        const double bktWidth) {
    // Setup starting & current timestamp for this rung.
    rStartTS = rCurrTS = minTS;
    // Ensure bucket width is not ridiculously small
    bucketWidth = bktWidth;
    currBucket  = 0;  // current bucket in this rung.
    DEBUG(std::cout << "bucketWidth = " << bucketWidth << std::endl);
    ASSERT(bucketWidth > 0);
    ASSERT(rungEventCount == 0);
    // Move events from given bucket into buckets in this Rung.
    move(std::move(bkt));
}

void
muse::TwoTierRung::move(TwoTierBucket&& bkt) {
    DEBUG(std::cout << "Adding " << bkt.size() << " events to rung\n");
    for (BktEventList& list : bkt.getSubBuckets()) {
        // Add all events from sub-buckets to various buckets in this rung.
        while (!list.empty()) {
            // Remove event from the top linked list.
            muse::Event* event = list.back();
            list.pop_back();
            // Add to the appropriate bucket in this rung using a
            // helper method in this class.
            enqueue(event);
        }
    }
    // Reset bucket counters as we have moved all the events out
    bkt.resetCount();
    DEBUG(validateEventCounts());
}

void
muse::TwoTierRung::move(OneTierBottom&& bottom, const Time rStart,
                        const double bktWidth) {
    rStartTS = rCurrTS = rStart;
    // Ensure bucket width is not ridiculously small
    bucketWidth = bktWidth;
    currBucket  = 0;  // current bucket in this rung.    
    ASSERT(rungEventCount == 0);
    DEBUG(std::cout << "bucketWidth = " << bucketWidth << std::endl);
    ASSERT(bucketWidth > 0);
    ASSERT(rungEventCount == 0);
    // Move events from bottom into buckets in this Rung.
    DEBUG(std::cout << "Adding " << bottom.size() << " events to rung\n");
    // OneTierBottom is sorted list of events. To preserve sorting
    // (and keep sorting overheads down), we will process events in
    // reverse order.
    for (OneTierBottom::reverse_iterator curr = bottom.rbegin();
         (curr != bottom.rend()); curr++) {
        // Add to the appropriate bucket in this rung using a
        // helper method in this class.
        enqueue(*curr);
    }
    // Finally clear out the events in bottom.
    bottom.clear();
    DEBUG(validateEventCounts());    
}
                                        

bool
muse::TwoTierRung::canContain(muse::Event* event) const {
    const muse::Time recvTime = event->getReceiveTime();
    const int bucketNum = (recvTime - rStartTS) / bucketWidth;
    return ((bucketNum >= (int) currBucket) && (recvTime >= rCurrTS));
}

void
muse::TwoTierRung::enqueue(muse::Event* event) {
    ASSERT(event != NULL);
    ASSERT(event->getReceiveTime() >= getCurrTime());
    // Compute bucket for this event based on equation #2 in LQ paper.
    size_t bucketNum = (event->getReceiveTime() - rStartTS) / bucketWidth;
    ASSERT(bucketNum >= currBucket);
    if (bucketNum > MAX_BUCKETS) {
        // Rebucket events in this rung to ensure the bucket count
        // does not become too large.
        const double maxTS = rStartTS + (bucketNum * bucketWidth);
        const double newBktWidth = (maxTS - rStartTS) / MAX_BUCKETS;
        DEBUG({
                if (newBktWidth <= bucketWidth) {
                    std::cout << "Current bucketWidth = " << bucketWidth
                              << ", new bucketWidth   = " << newBktWidth
                              << std::endl;
                    std::cout << "rStartTS = " << rStartTS << ", bucketNum = "
                              << bucketNum << ", maxTS = " << maxTS
                              << ", rungEventCount = " << rungEventCount
                              << std::endl;
                }
        });
        ASSERT(newBktWidth >= bucketWidth);
        // Use suitable constructor to create new rung with events
        // redistributed from this rung.
        TwoTierRung rebucketed(std::move(*this), newBktWidth);
        *this = std::move(rebucketed);
        ASSERT(newBktWidth == bucketWidth);
        // Now recompute bcuket number based on the new bucket sizes
         bucketNum = (event->getReceiveTime() - rStartTS) / bucketWidth;
         ASSERT(bucketNum <= MAX_BUCKETS);
         LQ2T_STATS(numRedistr++);
         DEBUG(std::cout << "Redistributed events in rung\n");
    }
    if (bucketNum >= bucketList.size()) {
        // Ensure bucket list of sufficient size
        bucketList.resize(bucketNum + 1);
        // update variable to track maximum bucket count
        LQ2T_STATS(maxBkts = std::max(maxBkts, bucketList.size()));
    }
    ASSERT(bucketNum < bucketList.size());
    // Add event into appropriate bucket
    bucketList[bucketNum].push_back<SenderID>(event);
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
    for (size_t bktNum = currBucket; (bktNum < bucketList.size()); bktNum++) {
        if (!bucketList[bktNum].empty() &&
            (rStartTS + (bktNum + 1) * bucketWidth) >= sendTime) {
            // Have the bucket remove necessary event(s) and update stats
            numRemoved +=
                bucketList[bktNum].remove_after(sender, sendTime
                                                LQ2T_STATS(COMMA ceScanRung));
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
    for (size_t bktNum = currBucket; (bktNum < bucketList.size()); bktNum++) {
        if (!bucketList[bktNum].empty()) {
            // This stat needs to be tracked by the bucket and not here.
            LQ2T_STATS(ceScanRung += bucketList[bktNum].size());
            // Remove appropriate set of events.
            numRemoved += bucketList[bktNum].remove(receiver);
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

// The maximum number of rungs typically allowed in the ladder.  This
// value is set to 8 by default based on Tang et. al.  It can be set
// via command-line parameter --lq-max-rungs 8.
size_t muse::TwoTierLadderQueue::MaxRungs = 8;

void
muse::TwoTierLadderQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);    
    LQ2T_STATS({
            // Collect final bucket counts from the ladder
            for (size_t i = 0; (i < nRung); i++) {
                ladder[i].updateStats(avgBktCnt);
            }
            // Track final bucket counts per rung -- Note: for-loop
            // range is intentionally different from the one right above
            std::string rungBktSizes;
            for (const TwoTierRung& rung: ladder) {
                rungBktSizes += std::to_string(rung.getBucketListSize()) + "(";
                rungBktSizes += std::to_string(rung.getNumRedistr()) + ") ";
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
               << "\nNo cancel scans of bottom   : " << ceNoCanScanBot
               << "\nInserts into top            : " << insTop
               << "\nInserts into rungs          : " << insLadder
               << "\nInserts into bottom         : " << insBot
               << "\nMax rung count              : " << maxRungs
               << "\nAverage #buckets per rung   : " << avgBktCnt
               << "\nAverage bottom size         : " << botLen
               << "\nMax bottom size             : " << maxBotSize
               << "\nAverage bucket width        : " << avgBktWidth
               << "\nBottom to rung operations   : " << botToRung
               << "\nRung #bkts (#redistr)       : " << rungBktSizes
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
    // Try to see if the event fits in the ladder. nRung is max rung index
    size_t rung = 0;
    while ((rung < nRung) && !ladder[rung].canContain(event)) {
        ASSERT((rung == 0) || ladder[rung].empty() ||
               (ladder[rung - 1].getCurrTime() >= ladder[rung].getCurrTime()));
        rung++;
    }
    if (rung < nRung) {
        DEBUG(ASSERT(bottom.empty() ||
                     (event->getReceiveTime() > bottom.maxTime())));
        ladder[rung].enqueue(event);
        ladderEventCount++;  // Track events added to the ladder
        DEBUG(std::cout << "Added to rung " << rung  << "(max bottom: "
                        << bottom.maxTime() << "): " << *event << "\n");
        LQ2T_STATS(insLadder++);
        return;
    }
    // Event does not fit in the ladder. It must go into bottom.
    // However, to ensure good performance we must keep bottom short.
    if ((bottom.size() > LQ2T_THRESH) && (bottom.getTimeRange() > 0)) {
        // Move events from bottom into ladder rung
        rung = createRungFromBottom();
        ASSERT(rung == nRung - 1);
        ASSERT(rung < ladder.size());
        // Due to rollback-reprocessing the event may be even
        // earlier than the last rung we just created!
        if (ladder[rung].canContain(event)) {
            ladder[rung].enqueue(event);
            ladderEventCount++;
            DEBUG(std::cout << "Added to rung " << rung  << "(max bottom: "
                            << bottom.maxTime() << "): " << *event << "\n");
            LQ2T_STATS(insLadder++);
            return;
        }
    }
    // At this point, event must go into bottom, so enqueue it.
    bottom.enqueue(event);
    LQ2T_STATS(maxBotSize = std::max(maxBotSize, bottom.size()));
    DEBUG(ASSERT(!haveBefore(bottom.first_event()->getReceiveTime())));
    DEBUG(std::cout << "Added to bottom: " << *event << std::endl);
    LQ2T_STATS(insBot++);
}

// Implementation close to the version from the paper.
muse::TwoTierBucket&&
muse::TwoTierLadderQueue::recurseRung() {
    ASSERT(!empty());
    ASSERT(nRung > 0);
    ASSERT(!ladder.empty());
    ASSERT(nRung <= ladder.size());
    // Now the last rung in ladder is the rung that has the next
    // bucket of events.
    muse::Time bktTime    = 0;  // set by removeNextBucket call below
    TwoTierRung& lastRung = ladder[nRung - 1];
    TwoTierBucket&& bkt   = lastRung.removeNextBucket(bktTime);
    ASSERT(!bkt.empty());
    ASSERT(!ladder.empty());
    // Check and create new rung in the ladder if the bucket is large.
    if ((bkt.size() > LQ2T_THRESH) && (nRung < MaxRungs)) {
        // Note: Here bucket width can dip a bit low. But that is
        // needed to ensure consistent ladder setup.
        double bucketWidth = (lastRung.getBucketWidth() + bkt.size() -
                              1.0) / bkt.size();
        // Ensure bucketWidth is not so small that we get too many bcukets.
        if ((lastRung.getBucketWidth() / bucketWidth) >= MAX_BUCKETS) {
            bucketWidth = lastRung.getBucketWidth() / MAX_BUCKETS;
        }
        // Create a new rung in the ladder
        nRung++;
        if (nRung > ladder.size()) {
            ladder.push_back(TwoTierRung(std::move(bkt), bktTime, bucketWidth));
            ASSERT(nRung == ladder.size());
        } else {
            ladder[nRung - 1].move(std::move(bkt), bktTime, bucketWidth);
        }
        DEBUG(std::cout << "2. Bucket width: " << bucketWidth << std::endl);
        LQ2T_STATS(avgBktWidth += bucketWidth);
        LQ2T_STATS(maxRungs = std::max(maxRungs, nRung));
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
    if (ladderEventCount == 0) {   // nRung == -1
        if (top.empty()) {
            // There are no events in the ladder queue in this case
            ASSERT(empty());
            return;
        }
        // Move all events from top into buckets in first rung of the ladder!
        nRung++;
        ASSERT(nRung == 1);
        ladderEventCount += top.size();     // Track events in ladder
        // Move events to ladder
        if (nRung > ladder.size()) {
            ladder.push_back(TwoTierRung(std::move(top)));
            ASSERT(nRung == ladder.size());
        } else {
            ladder[nRung - 1].move(std::move(top), top.getMinTime(),
                                   top.getBucketWidth());
        }
        // Reset top counters and update the values of topStart for
        // next Epoch
        top.reset(top.getMaxTime());
        LQ2T_STATS(maxRungs = std::max(maxRungs, nRung));
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
    LQ2T_STATS(maxBotSize = std::max(maxBotSize, bottom.size()));
    DEBUG(ASSERT(!haveBefore(bottom.first_event()->getReceiveTime())));
    LQ2T_STATS(botLen += bottom.size());
    // Clear out the rungs if we have used-up the last bucket in the ladder.
    while (nRung > 0 && ladder[nRung - 1].empty()) {
        LQ2T_STATS(ladder[nRung - 1].updateStats(avgBktCnt));
        nRung--;  // Logically remove rung from ladder
    }
}

int
muse::TwoTierLadderQueue::createRungFromBottom() {
    ASSERT(!bottom.empty());
    ASSERT(bottom.getTimeRange() > 0);
    DEBUG(std::cout << "Moving events from bottom to a new rung. Bottom has "
                    << bottom.size() << " events. Bottom time range = "
                    << bottom.getTimeRange() << std::endl);
    // Compute the start time and bucket width for the rung.  Note
    // that with rollbacks, ladder can be empty and that situation
    // needs to be handled.
    DEBUG(std::cout << "Ladder is empty: " << std::boolalpha << ladder.empty()
                    << ". nRung = " << nRung << ", ladder.size() = "
                    << ladder.size() << std::endl);
    // const double bucketWidth = ((nRung == 0) ? bottom.getBucketWidth() :
    //                            ladder[nRung - 1].getBucketWidth());
    const double bucketWidth = bottom.getBucketWidth();
    // The paper computes rStart as RCur[NRung-1].  However, due to
    // rollback-reprocessing the bottom may have events that are below
    // RCur[NRung-1].  Consequently, we use the minimum of the two
    // values as as rstart
    const Time ladBkTime = ((nRung > 0) ? ladder[nRung - 1].getCurrTime() :
                            TIME_INFINITY);
    const Time rStart = std::min(ladBkTime,
                                 bottom.first_event()->getReceiveTime()); 
    ASSERT(rStart < ladBkTime);
    ASSERT(bottom.maxTime() < ladBkTime);
    ASSERT(bottom.maxTime() <= top.getStartTime());
    // Create a new rung and add it to the ladder.
    DEBUG(std::cout << "Moving bottom to rung. Events: " << bottom.size()
                    << ", rStart = " << rStart << ", bucketWidth = "
                    << (bucketWidth / bottom.size()) << std::endl);
    ladderEventCount += bottom.size();  // Update ladder event count
    LQ2T_STATS(botToRung += bottom.size());
    // double bktWidth = (bucketWidth + bottom.size() - 1.0) / bottom.size();
    double bktWidth = bucketWidth;
    DEBUG(std::cout << "bktWidth = " << bktWidth << std::endl);
    // Add rung and move move bottom into the last rung of the ladder.
    nRung++;
    if (nRung > ladder.size()) {
        ladder.push_back(TwoTierRung(std::move(bottom), rStart, bktWidth));
        ASSERT(nRung == ladder.size());
    } else {
        ladder[nRung - 1].move(std::move(bottom), rStart, bktWidth);
    }
    DEBUG(std::cout << "1. Bucket width: " << bktWidth << std::endl);
    LQ2T_STATS(avgBktWidth += bktWidth);
    LQ2T_STATS(maxRungs = std::max(maxRungs, nRung)); // Track max rungs
    ASSERT(bottom.empty());
    return nRung - 1;
}

int
muse::TwoTierLadderQueue::remove_after(muse::AgentID sender,
                                       const Time sendTime) {
    // Check and cancel entries in top rung.
    int numRemoved = top.remove_after(sender, sendTime
                                      LQ2T_STATS(COMMA ceScanTop));
    LQ2T_STATS(ceTop += numRemoved);
    // Cancel out events in each rung of the ladder.
    for (size_t rung = 0; (rung < nRung); rung++) {
        const int rungEvtRemoved =
            ladder[rung].remove_after(sender, sendTime
                                      LQ2T_STATS(COMMA ceScanLadder));
        ladderEventCount  -= rungEvtRemoved;
        numRemoved        += rungEvtRemoved;
        LQ2T_STATS(ceLadder += rungEvtRemoved);
    }
    // Clear out the rungs in ladder that are now empty after event
    // cancellations.
    while (nRung > 0 && ladder[nRung - 1].empty()) {
        LQ2T_STATS(ladder[nRung - 1].updateStats(avgBktCnt));
        nRung--;  // Logically remove rung from ladder
    }    
    // Save original size of bottom to track stats.
    LQ2T_STATS(const size_t botSize  = bottom.size());
    // Cancel events from bottom.
    const int botRemoved  = bottom.remove_after(sender, sendTime);
    if (botRemoved > -1) {
        numRemoved += botRemoved;
        // Update statistics counters
        LQ2T_STATS(ceBot += botRemoved);
        LQ2T_STATS(ceScanBot += botSize);
        LQ2T_STATS((botRemoved == 0) ? (ceNoCanScanBot += botSize) : 0);
    }
    return numRemoved;
}

// This method is purely for debugging. So performance is not
// important
bool
muse::TwoTierLadderQueue::haveBefore(const Time recvTime,
                                     const bool checkBottom) const {
    // Check top
    if (top.haveBefore(recvTime)) {
        std::cout << "Top has event that is <= " << recvTime << std::endl;
        prettyPrint(std::cout);
        return true;
    }
    // Check each rung of the ladder
    for (size_t rung = 0; (rung < nRung); rung++) {
        if (ladder[rung].haveBefore(recvTime)) {
            std::cout << "Rung #" << rung << " has event that is <= "
                      << recvTime << std::endl;
            prettyPrint(std::cout);
            return true;
        }
    }
    // Check bottom rung.
    if (checkBottom && bottom.haveBefore(recvTime)) {
        std::cout << "Bottom has event that is <= " << recvTime << std::endl;
        prettyPrint(std::cout);
        return true;
    }
    // When control drops here it mean the whole 2-tier ladder queue
    // does not have an event with timestamp lower than recvTime.
    return false;
}

// ------------------------[  EventQueue implementation ]----------------------

void*
muse::TwoTierLadderQueue::addAgent(muse::Agent* agent) {
    UNUSED_PARAM(agent);
    return NULL;  // 2-tier queue has no cross-references to store in agent
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
    LQ2T_STATS(const size_t botSize = bottom.size());
    const int botRemoved  = bottom.remove(receiver);
    LQ2T_STATS(ceScanBot += botSize);
    LQ2T_STATS((botRemoved == 0) ? (ceNoCanScanBot += botSize) : 0);
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
    return bottom.first_event();
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
    increaseReference(event);  // Use base class helper method
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
    std::cout << "Ladder (rungs=" << nRung << ", size="
              << ladder.size() << "):\n";
    for (size_t i = 0; (i < nRung); i++) {
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
