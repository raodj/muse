#ifndef LADDER_QUEUE_CPP
#define LADDER_QUEUE_CPP

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
#include "LadderQueue.h"

#define MAX_BUCKETS 100
#define MIN_BUCKET_WIDTH 0.1

// ----------------------[ ListBucket methods ]-----------------------

muse::ListBucket::~ListBucket() {
    for (auto& event : list) {
        event->decreaseReference();
    }
}

int
muse::ListBucket::remove_after(muse::AgentID sender, const Time sendTime) {
    size_t removedCount = 0;
    ListBucket::iterator next = list.before_begin();
    ListBucket::iterator curr = next++;
    while (next != list.end()) {
        muse::Event* const event = *next;
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sendTime)) {
            list.erase_after(curr);
            event->decreaseReference();
            removedCount++;
            next = curr;
            next++;
        } else {
            next++;
            curr++;
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

bool
muse::ListBucket::haveBefore(const Time recvTime) const {
    EventList::const_iterator next = list.begin();
    while (next != list.end()) {
        muse::Event* const event = *next;
        if (event->getReceiveTime() <= recvTime) {
            return true;
        }
        next++;
    }
    return false;
}

int
muse::ListBucket::remove(muse::AgentID receiver) {
    size_t removedCount = 0;
    ListBucket::iterator next = list.before_begin();
    ListBucket::iterator curr = next++;
    while (next != list.end()) {
        muse::Event* const event = *next;
        if (event->getReceiverAgentID() == receiver) {
            list.erase_after(curr);
            event->decreaseReference();
            removedCount++;
            next = curr;
            next++;
        } else {
            next++;  // on to the next events
            curr++;  // track both iterators.
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;    
}

// ----------------------[ VectorBucket methods ]-----------------------

muse::VectorBucket::~VectorBucket() {
    for (auto& event : list) {
        event->decreaseReference();
    }
}

void
muse::VectorBucket::push_back(VectorBucket&& bucket) {
    list.reserve(list.size() + bucket.size());
    list.insert(list.end(), bucket.list.begin(), bucket.list.end());
    count += bucket.size();
    bucket.list.clear();
    bucket.count = 0;
}

int
muse::VectorBucket::remove_after(muse::AgentID sender, const Time sendTime) {
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
            curr++;  // on to the next event in the list
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

int
muse::VectorBucket::remove_after_sorted(muse::AgentID sender,
                                        const Time sendTime) {
    // Since bucket is sorted we can shortcircuit scan if last event's
    // time is less-or-equal to sendTime.
    if (list.empty() || (sendTime >= list.front()->getReceiveTime())) {
        return 0;  // this bucket does not have events to be cancelled.
    }
    size_t removedCount = 0;
    EventVector::iterator curr = list.begin();
    while (curr != list.end()) {
        muse::Event* const event = *curr;
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sendTime)) {
            // Free-up event.
            event->decreaseReference();
            removedCount++;
            // To preserved sorted order erase from list correctly.
            curr = list.erase(curr);
        } else {
            curr++;  // on to the next event in the list
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

int
muse::VectorBucket::remove(muse::AgentID receiver) {
    size_t removedCount = 0;
    size_t curr = 0;
    while (curr < list.size()) {
        muse::Event* const event = list[curr];
        if (event->getReceiverAgentID() == receiver) {
            list.erase(list.begin() + curr);
            event->decreaseReference();
            removedCount++;
        } else {
            curr++;
        }
    }
    count -= removedCount;  // Track remaining events
    return removedCount;
}

bool
muse::VectorBucket::haveBefore(const Time recvTime) const {
    EventVector::const_iterator next = list.begin();
    while (next != list.end()) {
        muse::Event* const event = *next;
        if (event->getReceiveTime() <= recvTime) {
            return true;
        }
        next++;
    }
    return false;
}

// ---------------------------[ Top methods ]-----------------------------

muse::Top::~Top() {
}

void
muse::Top::reset(const Time startTime) {
    minTS    = TIME_INFINITY;
    maxTS    = 0;
    topStart = startTime;
}

void
muse::Top::add(muse::Event* event) {
    events.push_front(event);
    minTS = std::min(minTS, event->getReceiveTime());
    maxTS = std::max(maxTS, event->getReceiveTime());
}

int
muse::Top::remove_after(muse::AgentID sender, const Time sendTime) {
    return events.remove_after(sender, sendTime);
}

int
muse::Top::remove(muse::AgentID receiver) {
    return events.remove(receiver);
}

// ---------------------------[ Bottom methods ]-----------------------------

void
muse::Bottom::enqueue(ListBucket&& bucket, ListBucket&) {
    // Note that pop_front must be O(1) here -- which it is since
    // bucket is a linked list.
    while (!bucket.empty()) {
        enqueue(bucket.pop_front());
    }
}

void
muse::Bottom::enqueue(VectorBucket&& bucket, VectorBucket&botList) {
    // For vector-to-vector copy use different strategy for buckets
    botList.push_back(std::move(bucket));
    // Now sort the whole bottom O(n*log(n)) operation
    std::sort(botList.begin(), botList.end(), Bottom::compare);
}

void
muse::Bottom::enqueue(muse::Event* event, ListBucket& botList) {
    if (botList.empty() || !compare(event, botList.front())) {
        // Empty list or event is smaller than head.  Add event to the
        // front of the list.
        botList.push_front(event);
    } else {
        // Insert event in sorted list of events. For this we need to
        // search for the correct insertion location.
        ASSERT(compare(event, botList.front()));
        ListBucket::iterator next = botList.begin();
        ListBucket::iterator prev = next++;
        while ((next != botList.end()) && compare(event, *next)) {
            prev = next++;
        }
        botList.insert_after(prev, event);
    }
    DEBUG(validate());
}

void
muse::Bottom::enqueue(muse::Event* event, VectorBucket& botList) {
    VectorBucket::reverse_iterator iter =
        std::upper_bound(botList.rbegin(), botList.rend(), event, revCompare);
    botList.insert_after(iter, event);
    /*    
    if (botList.empty() || !compare(event, botList.front())) {
        // Empty list or event is smaller than head.  Add event to the
        // front of the list.
        botList.push_front(event);
    } else {
        // Insert event in sorted list of events. For this we need to
        // search for the correct insertion location.
        ASSERT(compare(event, botList.front()));
        VectorBucket::reverse_iterator next = botList.rbegin();
        while ((next != botList.rend()) && compare(event, *next)) {
            next++;
        }
        botList.insert_after(next, event);
    }
    */
    DEBUG(validate());
}

double
muse::Bottom::getBucketWidth() const {
    if (empty()) {
        return 0;
    }
    ASSERT(sel.front() != NULL);
    ASSERT(sel.back()  != NULL);
    const double maxTS = sel.front()->getReceiveTime();
    const double minTS = sel.back()->getReceiveTime();
    return (maxTS - minTS + size() - 1.0) / sel.size();
}

int
muse::Bottom::remove(muse::AgentID receiver) {
    return sel.remove(receiver);
}

void
muse::Bottom::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (empty()) {
        return;
    }
    const muse::Event*  nextEvt  = front();
    const muse::AgentID receiver = nextEvt->getReceiverAgentID();
    const muse::Time    currTime = nextEvt->getReceiveTime();

    do {
        muse::Event* event = pop_front();
        events.push_back(event);
        nextEvt = (!empty() ? front() : NULL);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() && (nextEvt->getReceiverAgentID() == receiver) &&
             TIME_EQUALS(nextEvt->getReceiveTime(), currTime));
    DEBUG(validate());
}

muse::Time
muse::Bottom::findMinTime() const {
    if (empty()) {
        return TIME_INFINITY;
    }
    return sel.front()->getReceiveTime();
}

muse::Time
muse::Bottom::maxTime() const {
    if (empty()) {
        return TIME_INFINITY;
    }
    /*
    Bucket::const_iterator next = sel.cbegin();
    Bucket::const_iterator prev = next++;
    while (next != sel.cend()) {
        prev = next++;
    }
    return (*prev)->getReceiveTime();
    */
    return sel.back()->getReceiveTime();
}

void
muse::Bottom::validate() {
    if (sel.empty()) {
        return;
    }
    Bucket::iterator next = sel.begin();
    Bucket::iterator prev = next++;
    while ((next != sel.end()) &&
           ((*next)->getReceiveTime() >= (*prev)->getReceiveTime())) {
        prev = next++;
    }
    if (next != sel.end()) {
        std::cout << "Error in LadderQueue.Bottom: Event " << **next
                  << " was found after " << **prev << std::endl;
    }
    ASSERT( next == sel.end() );
}

// -----------------------[ HeapBottom methods ]---------------------------

void
muse::HeapBottom::enqueue(Bucket&& bucket) {
    // Note that pop_front must be O(1) here -- which it is since
    // bucket is a linked list.  Due to bulk adding, ensure that the
    // heap container has enough capacity.
    sel.reserve(sel.size() + bucket.size() + 1);
    // Add all the events to the conainer.
    while (!bucket.empty()) {
        sel.push_back(bucket.pop_front());
    }
    // Restore heap properties for the queue.
    std::make_heap(sel.begin(), sel.end(), Bottom::compare);
}

void
muse::HeapBottom::enqueue(muse::Event* event) {
    ASSERT(sel.empty() || (front()->getReceiveTime() <= findMinTime()));
    sel.push_back(event);
    std::push_heap(sel.begin(), sel.end(), Bottom::compare);
}

muse::Event*
muse::HeapBottom::pop_front() {
    std::pop_heap(sel.begin(), sel.end(), Bottom::compare);
    muse::Event* retVal = sel.back();
    sel.pop_back();
    return retVal;
}

int
muse::HeapBottom::remove_after(muse::AgentID sender, const Time sendTime) {
    // Prior to Dec 23 2016, this method would copy events to be
    // retained into a temporary vector and make heap again.  This
    // approach was too slow for larger list.  So instead, this method
    // removes events one at a time from the heap, similar to the one
    // done in BinaryHeapWrapper::removeFutureEvents.
    size_t removedCount = 0;
    long currIdx        = sel.size() - 1;
    while (!sel.empty() && (currIdx >= 0)) {
        ASSERT(currIdx < (long) sel.size());
        muse::Event* const event = sel[currIdx];
        ASSERT(event != NULL);
        // An event is deleted only if the *sent* time is greater than
        // the sendTime and if the event is from same sender        
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sendTime)) {
            // This event needs to be removed.
            event->decreaseReference();
            removedCount++; 
            // Now it is time to patchup the hole and fix up the heap.
            // To patch-up we move event from the bottom up to this
            // slot and then fix-up the heap.
            sel[currIdx] = sel.back();
            sel.pop_back();
            // Fix-up the heap using helper method.
            EventQueue::fixHeap(sel, currIdx, Bottom::compare);
            // Update the current index so that it is within bounds.
            currIdx = std::min<long>(currIdx, sel.size() - 1);
        } else {
            // Check the previous element in the vector to see if that
            // is a candidate for cancellation.
            currIdx--;
        }
    }
    // Return number of events canceled to track statistics.
    return removedCount;
}

int
muse::HeapBottom::remove(muse::AgentID receiver) {
    // Copy events to be retained into a temporary vector and finally
    // swap it with sel.
    size_t removedCount = 0;
    EventVector retained;
    retained.reserve(sel.size());
    for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
        muse::Event* const event = *curr;
        if (event->getReceiverAgentID() == receiver) {
            event->decreaseReference();
            removedCount++;
        } else {
            retained.push_back(event);
        }
    }
    // Update the sel with list of retained events
    sel.swap(retained);
    std::make_heap(sel.begin(), sel.end(), Bottom::compare);
    return removedCount;
}

void
muse::HeapBottom::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (sel.empty()) {
        return;
    }
    
    const muse::Event*  nextEvt    = front();
    const muse::AgentID receiver   = nextEvt->getReceiverAgentID();
    const muse::Time    currTime   = nextEvt->getReceiveTime();

    do {
        muse::Event* event = pop_front();
        events.push_back(event);
        nextEvt = (!empty() ? front() : NULL);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() && (nextEvt->getReceiverAgentID() == receiver) &&
             TIME_EQUALS(nextEvt->getReceiveTime(), currTime));
    DEBUG(validate());
}

muse::Time
muse::HeapBottom::maxTime() const {
    if (empty()) {
        return TIME_INFINITY;
    }
    muse::Time maxTime = front()->getReceiveTime();
    for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
        maxTime = std::max(maxTime, (*curr)->getReceiveTime());
    }
    return maxTime;
}

muse::Time
muse::HeapBottom::findMinTime() const {
    if (empty()) {
        return 0;
    }
    muse::Time minTime = front()->getReceiveTime();
    // for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
    //     minTime = std::min(minTime, (*curr)->getReceiveTime());
    // }
    return minTime;
}

bool
muse::HeapBottom::haveBefore(const Time recvTime) const {
    if (empty()) {
        return false;
    }
    for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
        if ((*curr)->getReceiveTime() <= recvTime) {
            return true;
        }
    }
    return false;
}

void
muse::HeapBottom::validate() {
    // Heap's are implemented using standard C++ algorithms and
    // consequently no special validation is deemed necessary.
}

void
muse::HeapBottom::print(std::ostream& os) const {
    os << "Bottom:";
    for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
        os << " " << (*curr)->getReceiveTime();
    }
    os << std::endl;
}

// -----------------------[ MultiSetBottom methods ]---------------------------

void
muse::MultiSetBottom::enqueue(Bucket&& bucket) {
    // Note that pop_front must be O(1) here -- which it is since
    // bucket is a linked list.
    // Add all the events to the conainer.
    while (!bucket.empty()) {
        sel.insert(bucket.pop_front());
    }
}

void
muse::MultiSetBottom::enqueue(muse::Event* event) {
    ASSERT(sel.empty() || (front()->getReceiveTime() <= findMinTime()));
    sel.insert(event);
}

muse::Event*
muse::MultiSetBottom::pop_front() {
    muse::Event* retVal = front();
    sel.erase(sel.begin());
    return retVal;
}

int
muse::MultiSetBottom::remove_after(muse::AgentID sender, const Time sendTime) {
    // Since MutliSet events are sorted based on receive time there is
    // only simple sanity checks we can do here...
    if (sendTime > maxTime()) {
        // Since max event time is greater than sentTime the bottom
        // cannot have an event to be cancelled.
        return 0;
    }
    size_t removedCount = 0;
    EventMultiSet::iterator currIdx = sel.begin();
    while (!sel.empty() && (currIdx != sel.end())) {
        muse::Event* const event = *currIdx;
        ASSERT(event != NULL);
        // An event is deleted only if the *sent* time is greater than
        // the sendTime and if the event is from same sender        
        if ((event->getSenderAgentID() == sender) &&
            (event->getSentTime() >= sendTime)) {
            // This event needs to be removed.
            event->decreaseReference();
            removedCount++;
            currIdx = sel.erase(currIdx);
        } else {
            // Check the next event to see it is a candidate for
            // cancellation.
            currIdx++;
        }
    }
    // Return number of events canceled to track statistics.
    return removedCount;
}

int
muse::MultiSetBottom::remove(muse::AgentID receiver) {
    size_t removedCount = 0;
    EventMultiSet::iterator currIdx = sel.begin();
    while (!sel.empty() && (currIdx != sel.end())) {
        Event* const event = *currIdx;
        if (event->getReceiverAgentID() == receiver) {
            event->decreaseReference();
            removedCount++;
            currIdx = sel.erase(currIdx);
        } else {
            currIdx++;
        }
    }
    return removedCount;
}

void
muse::MultiSetBottom::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (sel.empty()) {
        return;
    }
    
    const muse::Event*  nextEvt    = front();
    const muse::AgentID receiver   = nextEvt->getReceiverAgentID();
    const muse::Time    currTime   = nextEvt->getReceiveTime();

    do {
        muse::Event* event = pop_front();
        events.push_back(event);
        nextEvt = (!empty() ? front() : NULL);
        DEBUG(std::cout << "Delivering: " << *event << std::endl);
    } while (!empty() && (nextEvt->getReceiverAgentID() == receiver) &&
             TIME_EQUALS(nextEvt->getReceiveTime(), currTime));
    DEBUG(validate());
}

muse::Time
muse::MultiSetBottom::maxTime() const {
    if (empty()) {
        return TIME_INFINITY;
    }
    muse::Time maxTime = (*sel.crbegin())->getReceiveTime();
    ASSERT(maxTime >= front()->getReceiveTime());
    return maxTime;
}

muse::Time
muse::MultiSetBottom::findMinTime() const {
    if (empty()) {
        return 0;
    }
    muse::Time minTime = front()->getReceiveTime();
    ASSERT(minTime <= maxTime());
    return minTime;
}

bool
muse::MultiSetBottom::haveBefore(const Time recvTime) const {
    if (empty()) {
        return false;
    }
    for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
        if ((*curr)->getReceiveTime() <= recvTime) {
            return true;
        }
    }
    return false;
}

void
muse::MultiSetBottom::validate() {
    // MultiSet are implemented using standard C++ algorithms and
    // consequently no special validation is deemed necessary.
}

void
muse::MultiSetBottom::print(std::ostream& os) const {
    os << "MultiSetBottom:";
    for (auto curr = sel.begin(); (curr != sel.end()); curr++) {
        os << " " << (*curr)->getReceiveTime();
    }
    os << std::endl;
}

// ---------------------------[  Rung methods ]-----------------------------

muse::Rung::Rung(Top& top) : Rung(std::move(top.events), top.minTS,
                                  std::max(MIN_BUCKET_WIDTH, top.getBucketWidth())) {
    // Reset top counters and update the values of topStart for next Epoch
    top.reset(top.maxTS);
}

muse::Rung::Rung(Bucket&& bkt, const Time minTS, const double bktWidth) :
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
    while (!bkt.empty()) {
        // Remove event from the top linked list.
        muse::Event* event = bkt.pop_front();
        // Add to the appropriate bucket in this rung
        enqueue(event);
    }
    DEBUG(validateEventCounts());
}

muse::Rung::Rung(EventVector&& list, const Time minTS, const double bktWidth) :
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
    DEBUG(std::cout << "Adding " << list.size() << " events to rung\n");
    for (auto curr = list.begin(); (curr != list.end()); curr++) {
        // Add to the appropriate bucket in this rung
        enqueue(*curr);
    }
    // Clear out entries in the supplied list
    list.clear();
    DEBUG(validateEventCounts());
}

muse::Rung::Rung(EventMultiSet&& set, const Time minTS, const double bktWidth) :
    rStartTS(minTS), rCurrTS(minTS), bucketWidth(bktWidth), currBucket(0),
    rungEventCount(0) {
    // Initialize variable to track maximum bucket count
    LQ_STATS(maxBkts = 0);    
    // Ensure bucket width is not ridiculously small
    bucketWidth = std::max(MIN_BUCKET_WIDTH, bucketWidth);
    DEBUG(std::cout << "bucketWidth = " << bucketWidth << std::endl);
    ASSERT(bucketWidth > 0);
    ASSERT(rungEventCount == 0);
    // Move events from given set into buckets in this Rung.
    DEBUG(std::cout << "Adding " << list.size() << " events to rung\n");
    for (auto curr = set.begin(); (curr != set.end()); curr++) {
        // Add to the appropriate bucket in this rung
        enqueue(*curr);
    }
    // Clear out entries in the supplied list
    set.clear();
    DEBUG(validateEventCounts());
}

bool
muse::Rung::canContain(muse::Event* event) const {
    const muse::Time recvTime = event->getReceiveTime();
    const int bucketNum = (recvTime - rStartTS) / bucketWidth;
    return ((bucketNum >= (int) currBucket) && (recvTime >= rStartTS));
}

void
muse::Rung::enqueue(muse::Event* event) {
    ASSERT(event->getReceiveTime() >= getCurrTime());
    // Compute bucket for this event based on equation #2 in paper.
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
    bucketList[bucketNum].push_front(event);
    // Track number of events added to this Rung
    rungEventCount++;
}

muse::Bucket&&
muse::Rung::removeNextBucket(muse::Time& bktTime) {
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
muse::Rung::remove_after(muse::AgentID sender, const Time sendTime
                         LQ_STATS(COMMA Avg& ceScanRung)) {
    if (empty() || (sendTime > getMaxRungTime())) {
        return 0;
    }
    int numRemoved   = 0;
    for (size_t bucketNum = currBucket; (bucketNum < bucketList.size());
         bucketNum++) {
        if (!bucketList[bucketNum].empty() &&
            (rStartTS + (bucketNum + 1) * bucketWidth) >= sendTime) {
            LQ_STATS(ceScanRung += bucketList[bucketNum].size());
            numRemoved += bucketList[bucketNum].remove_after(sender, sendTime);
        }
    }
    rungEventCount -= numRemoved;
    DEBUG(validateEventCounts());
    return numRemoved;
}

int
muse::Rung::remove(muse::AgentID receiver
                   LQ_STATS(COMMA Avg& ceScanRung)) {
    if (empty()) {
        return 0;
    }
    int numRemoved = 0;
    for (size_t bucketNum = currBucket; (bucketNum < bucketList.size());
         bucketNum++) {
        if (!bucketList[bucketNum].empty()) {
            LQ_STATS(ceScanRung += bucketList[bucketNum].size());
            numRemoved += bucketList[bucketNum].remove(receiver);
        }
    }
    rungEventCount -= numRemoved;
    DEBUG(validateEventCounts());
    return numRemoved;
}

void
muse::Rung::validateEventCounts() const {
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

void
muse::Rung::prettyPrint(std::ostream& os) const {
    // Compute minimum, maximum, empty, and average bucket sizes.
    size_t minBkt = -1U, maxBkt = 0, emptyBkt = 0, sizeSum = 0;
    for (const Bucket& bkt : bucketList) {
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

void
muse::Rung::updateStats(Avg& avgBktCnt) const {
    LQ_STATS(avgBktCnt += maxBkts);
}

bool
muse::Rung::haveBefore(const Time recvTime) const {
    for (size_t i = 0; (i < bucketList.size()); i++) {
        if (bucketList[i].haveBefore(recvTime)) {
            return true;
        }
    }
    return false;
}

// ------------------------[  LadderQueue methods ]----------------------

muse::LadderQueue::~LadderQueue() {
    // Nothing else to be dne here.
}

void
muse::LadderQueue::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);    
    LQ_STATS({
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
               << "\nMax bottom size             : " << maxBotSize
               << "\nAverage bucket width        : " << avgBktWidth
               << "\nBottom to rung operations   : " << botToRung
               << "\nCompare estimate            : " << comps
               << std::endl;
    });
}

void
muse::LadderQueue::enqueue(muse::Event* event) {
    if (top.getStartTime() < event->getReceiveTime()) {
        DEBUG(std::cout << "Added to top: " << *event << std::endl);
        top.add(event);
        LQ_STATS(insTop++);
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
        LQ_STATS(insLadder++);
        return;
    }
    // Event does not fit in the ladder. Must go into bottom
    if ((bottom.size() > THRESH) && (bottom.getTimeRange() > 0)) {
        // (ladder.empty() || (ladder.back().getBucketWidth() > MIN_BUCKET_WIDTH))) {
        // Move events from bottom into ladder rung
        rung = createRungFromBottom();
        ASSERT(rung < ladder.size());
        // Due to rollback-reprocessing the event may be even
        // earlier than the last rung we just created!
        if (ladder[rung].canContain(event)) {
            ladder[rung].enqueue(event);
            ladderEventCount++;
            DEBUG(std::cout << "Added to rung " << rung  << "(max bottom: "
                            << bottom.maxTime() << "): " << *event << "\n");
            LQ_STATS(insLadder++);
            return;
        }
    }
    // At this point bottom must be able to contain the event, so
    // enqueue it.
    bottom.enqueue(event);
    LQ_STATS(maxBotSize = std::max(maxBotSize, bottom.size()));    
    if (bottom.size() > 50000) {
        std::cerr << "Warning: Bottom is very large: size: "
                  << bottom.size() << ".  Min event timestamp: "
                  << bottom.findMinTime() 
                  << ", Max event time: " << bottom.maxTime()
                  << ", ladder size: " << ladder.size()
                  << " with events: " << ladderEventCount
                  << std::endl;
        prettyPrint(std::cerr);
        ASSERT(ladder.empty());
    }
    
    DEBUG(ASSERT(!haveBefore(bottom.front()->getReceiveTime())));
    DEBUG(std::cout << "Added to bottom: " << *event << std::endl);
    LQ_STATS(insBot++);
}

int
muse::LadderQueue::createRungFromBottom() {
    ASSERT(!bottom.empty());
    DEBUG(std::cout << "Moving events from bottom to a new rung. Bottom has "
          << bottom.size() << " events." << std::endl);
    // Compute the start time and bucket width for the rung.  Note
    // that with rollbacks, ladder can be empty and that situation
    // needs to be handled.
    const double bucketWidth = (ladder.empty() ? bottom.getBucketWidth() :
                                ladder.back().getBucketWidth());
    // The paper computes rStart as RCur[NRung-1].  However, due to
    // rollback-reprocessing the bottom may have events that are below
    // RCur[NRung-1].  Consequently, we use the minimum of the two
    // values as as rstart
    const Time ladBkTime = (!ladder.empty() ? ladder.back().getCurrTime() :
                            TIME_INFINITY);
    const Time rStart = std::min(ladBkTime, bottom.front()->getReceiveTime());
    // Create a new rung and add it to the ladder.
    DEBUG(std::cout << "Moving bottom to rung. Events: " << bottom.size()
                    << ", rStart = " << rStart << ", bucketWidth = "
                    << (bucketWidth / bottom.size()) << std::endl);
    ladderEventCount += bottom.size();  // Update ladder event count
    LQ_STATS(botToRung += bottom.size());
    const double bktWidth = (bucketWidth + bottom.size() - 1.0) / bottom.size();
    // std::cout << "bktWidth = " << bktWidth << std::endl;
    ladder.push_back(Rung(std::move(bottom.sel), rStart, bktWidth));
    DEBUG(std::cout << "1. Bucket width: " << bktWidth << std::endl);
    LQ_STATS(avgBktWidth += bktWidth);
    LQ_STATS(maxRungs = std::max(maxRungs, ladder.size()));  // Track max rungs
    ASSERT(bottom.empty());
    return ladder.size() - 1;
}

muse::Bucket&&
muse::LadderQueue::recurseRung() {
    ASSERT(!empty());
    // Find the next non-empty bucket across multiple rungs in the
    // ladder
    /*
    LQ_STATS(bool isLastRung = true);
    while (!ladder.empty() && ladder.back().empty()) {
        // Track statistics if enabled.
        LQ_STATS({
                if (!isLastRung) {
                    ladder.back().updateStats(avgBktCnt);
                }
            });
        // Remove empty rung (i.e., NRung--) at the end.
        ladder.pop_back();
        // In the next iteration the rung to remove (if any) was not
        // the last rung in the ladder
        LQ_STATS(isLastRung = false);
    }
    */
    ASSERT(!ladder.empty());
    // Now the last rung in ladder is the rung that has the next
    // bucket of events.
    muse::Time bktTime = 0;
    Rung& lastRung     = ladder.back();    
    Bucket&& bkt       = lastRung.removeNextBucket(bktTime);

    if ((bkt.size() > THRESH) && (ladder.size() < MAX_RUNGS) &&
        (lastRung.getBucketWidth() > MIN_BUCKET_WIDTH)) {
        // Note: Here bucket width can dip below MIN_BUCKET_WIDTH. But
        // that is needed to ensure consistent ladder setup.
        const double bucketWidth = (lastRung.getBucketWidth() + bkt.size() -
                                    1.0) / bkt.size();
        ladder.push_back(Rung(std::move(bkt), bktTime, bucketWidth));
        DEBUG(std::cout << "2. Bucket width: " << bucketWidth << std::endl);
        LQ_STATS(avgBktWidth += bucketWidth);
        LQ_STATS(maxRungs = std::max(maxRungs, ladder.size()));
        return recurseRung();  // Recurse now looking at newly added rung
    }
    // Track events being removed from the ladder
    ladderEventCount -= bkt.size();
    ASSERT(ladderEventCount >= 0);
    // Return bucket being removed.
    return std::move(bkt);
}

muse::Event*
muse::LadderQueue::dequeue() {
    if (empty()) {
        return NULL;
    }
    if (!bottom.empty()) {
        muse::Event* retVal = bottom.pop_front();
        if (empty()) {
            // The whole queue is now empty. Reset the structure to
            // accomodate new events.
            top.reset();
        }
        return retVal;
    }
    // Since bottom is empty move events into bottom.
    populateBottom();
    ASSERT(!bottom.empty());
    return bottom.pop_front();
}

void
muse::LadderQueue::populateBottom() {
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
        ladderEventCount += top.size();  // Track events in ladder
        ladder.push_back(Rung(top));     // Move events into ladder
        LQ_STATS(maxRungs = std::max(maxRungs, ladder.size()));
        LQ_STATS(avgBktWidth += ladder.back().getBucketWidth());
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
    LQ_STATS(maxBotSize = std::max(maxBotSize, bottom.size()));
    ASSERT(!bottom.empty());
    DEBUG(ASSERT(!haveBefore(bottom.front()->getReceiveTime())));
    LQ_STATS(botLen += bottom.size());
    // Clear out the rungs if we have used-up the last bucket in the ladder.
    while (!ladder.empty() && ladder.back().empty()) {
        LQ_STATS(ladder.back().updateStats(avgBktCnt));
        ladder.pop_back();  // NRung--
    }
}

int
muse::LadderQueue::remove_after(muse::AgentID sender, const Time sendTime) {
    LQ_STATS(ceScanTop += top.size());    
    int numRemoved = top.remove_after(sender, sendTime);
    LQ_STATS(ceTop += numRemoved);

    for (auto& rung : ladder) {
        const int rungEvtRemoved =
            rung.remove_after(sender, sendTime LQ_STATS(COMMA ceScanLadder));
        ladderEventCount  -= rungEvtRemoved;
        numRemoved        += rungEvtRemoved;
        LQ_STATS(ceLadder += rungEvtRemoved);
    }
    LQ_STATS(ceScanBot  += bottom.size());
    const int botRemoved = bottom.remove_after(sender, sendTime);
    numRemoved          += botRemoved;
    LQ_STATS(ceBot      += botRemoved);
    return numRemoved;
}

bool
muse::LadderQueue::haveBefore(const Time recvTime,
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
muse::LadderQueue::addAgent(muse::Agent* agent) {
    UNUSED_PARAM(agent);
    return NULL;
}

void
muse::LadderQueue::removeAgent(muse::Agent* agent) {
    ASSERT( agent != NULL );
    const AgentID receiver = agent->getAgentID();
    // Remove events for agent from top
    LQ_STATS(ceScanTop += top.size());
    int numRemoved    = top.remove(receiver);
    LQ_STATS(ceTop += numRemoved);
    
    // Next remove events for agent from all the rungs in the ladder
    for (Rung& rung : ladder) {
        int rungEvtRemoved = rung.remove(agent->getAgentID()
                                         LQ_STATS(COMMA ceBot));
        ladderEventCount  -= rungEvtRemoved;
        numRemoved        += rungEvtRemoved;
        LQ_STATS(ceLadder += rungEvtRemoved);
    }
    // Finally remove events from bottom for the agent.
    LQ_STATS(ceScanBot  += bottom.size());
    const int botRemoved = bottom.remove(receiver);
    numRemoved          += botRemoved;
    LQ_STATS(ceBot      += botRemoved);
}


muse::Event*
muse::LadderQueue::front() {
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
muse::LadderQueue::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (empty()) {
        // No events to dequeue.
        return;
    }
    // We only dequeue from bottom. So ensure it has events in it.
    if (bottom.empty()) {
        populateBottom();
    }
    ASSERT(!bottom.empty());
    bottom.dequeueNextAgentEvents(events);
    ASSERT(!events.empty());
    DEBUG(ASSERT(!haveBefore(events.front()->getReceiveTime())));
}

void
muse::LadderQueue::enqueue(muse::Agent* agent, muse::Event* event) {
    UNUSED_PARAM(agent);
    event->increaseReference();
    enqueue(event);
}

void
muse::LadderQueue::enqueue(muse::Agent* agent, muse::EventContainer& events) {
    UNUSED_PARAM(agent);
    for (auto& curr : events) {
        enqueue(curr);
    }
    events.clear();
}

int
muse::LadderQueue::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                              const muse::Time sentTime) {
    UNUSED_PARAM(dest);
    return remove_after(sender, sentTime);
}

void
muse::LadderQueue::prettyPrint(std::ostream& os) const {
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
