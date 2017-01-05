#ifndef TWO_TIER_LADDER_QUEUE_H
#define TWO_TIER_LADDER_QUEUE_H

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

#include <forward_list>
#include <queue>
#include <vector>
#include <typeinfo>
#include <set>
#include "Avg.h"
#include "Event.h"
#include "EventQueue.h"

/** \file LadderQueue.h

    \brief Enhancement of LadderQueue to improve performance of
    Optimistic Parallel Simulations by minimizing rollbacks.

    The LadderQueue data structure is detailed in the following paper:

    W. Tang, R. Goh, and I. Thng, "Ladder queue: An O(1) priority
    queue structure for large-scale discrete event simulation", ACM
    TOMACS, Vol 15, Issue 3, Pages 175--204, July 2005. URL: 
    http://doi.acm.org/10.1145/1103323.1103324

    <p>One major disadvantage of the LadderQueue is that canceling
    events due to a rollback is expensive -- the whole queue has to be
    scanned.</p>

    <p>In order to reduce the overhead of scanning events for
    canceling, this TwoTierLadderQueue further subdivides each bucket
    in Top and ladder Rung to store events in a 2nd Tier based on
    their sender's ID.  It uses a simple hash function on the sender's
    AgentID to identify the 2nd tier bucket and enqueue's the event
    into that bucket.  Currently, the hash function is simply
    implemented as a modulo t2k, with t2k being an implementation
    dependent value.  Since second tier buckets (implemented as
    std::vector) are preallocated, Small t2k values increase 2nd tier
    bucket sizes increasing time for cancellation.  On the other hand
    if buckets are not used, then the space/time invested to create
    them can become an overhead.</p>

    <p>Note that the TwoTierLadderQueue would have very similar
    characteristics to LadderQueue in sequential or 1 process
    simulation as there are no rollbacks</p>
*/

// The maximum number of rungs permitted in the ladder.
#define MAX_RUNGS 8

// The minimum bucket timestamp
#define MIN_BUCKET_WIDTH 0.1

// Bucket size after which new rung is created in ladder
#define LQ2T_THRESH 100

/** \def LQ2T_STATS(x)

    \brief Define a convenient macro for conditionally compiling
    additional statistics collection regarding ladder queue.

    Define a custom macro LQ2T_STATS (note the all caps) macro to be
    used to conditionally compile in debugging code to generate
    detailed logs.  This helps to minimize code modification to insert
    and remove debugging messages.
*/
#define COMMA ,
#define LQ2T_STATS(x) x
// #define LQ2T_STATS(x)

BEGIN_NAMESPACE(muse)

/** A convenience alias for list of events maintained by a sub-bucket. */
using BktEventList = std::deque<muse::Event*>;
    
/** Alias to the data structure for holding a vector of sub-buckets in
    a TwoTierBucket.
*/
using SubBucketList = std::vector<BktEventList>;

/** A generic two tier bucket that is used for both Top and Rungs of
    the 2-tier ladderQ.

    <p>This bucket does not store events in it directly.  Instead it
    splits into t2k sub-buckets based on a simple hash function.
    Currently, the hash function is simply implemented as a modulo
    t2k, with t2k being an implementation dependent value.  Splitting
    the events into sub-buckets makes event cancellations easier.
    </p>
*/  
class TwoTierBucket {
public:
    /** The shared parameter indicating the number of sub-buckets to
        be used in each 2-tier bucket.  This value defaults to 32.  It
        is overriden by by command-line argument when ladder queue is
        used.
    */
    static int t2k;
    
    /** Constructor to create a bucket with fixed number (i.e., t2k)
        of tier-2 lists.
    */
    TwoTierBucket() : subBuckets(t2k), count(0) {}

    /** A move constructor to facilitate moving objects (if needed).

        \param[in,out] src The source object whose data is to be moved
        into this.  The source object does not contain any useful
        information after the move is complete.
    */
    TwoTierBucket(TwoTierBucket&& src) : subBuckets(std::move(src.subBuckets)),
                                         count(std::move(src.count)) {
        // Reset count in source to aid debugging.
        src.count = 0;
    }

    /** The destructor for this class.

        The destructor decreases the reference count on all the events
        in its list to free-up any pending events.
    */
    ~TwoTierBucket();

    /** The hash function used to distribute events into sub-buckets.

        \param[in] sender The sender's ID to be hashed.

        \return The hash value based on sender ID.  The return value,
        say hash, must be in the range 0 <= hash < t2k.
    */
    inline int hash(const muse::AgentID sender) const {
        // Use a simple hashing function for now.
        return (sender % t2k);
    }

    /** Add an event to this TwoTier bucket.

        The event is added to the sub-bucket identified using the hash
        function in this class.
        
        \param[in] event The event to be added to this bucket.  This
        method does not alter the refernece counts on events (as the
        top-level TwoTierLadderQueue performs the reference count
        management).
    */
    void push_back(muse::Event* event) {
        const size_t subBktIdx = hash(event->getSenderAgentID());
        subBuckets[subBktIdx].push_back(event);
        count++;
    }

    /** Move all the events from the given two tier bucket into this
        bucket.

        This method moves all the events from the sub-buckets in
        srcBkt to corresponding sub-buckets in this.
        
        \param[in,out] srcBkt The bucket from where the events are to
        be moved into this bucket.
    */
    void push_back(TwoTierBucket&& srcBkt);

    /** Helper method to move all events from a 2-tier bucket into a
        single list of events.

        This method is a convenience method that is used by the bottom
        tier to combine all the events from various sub-buckets into a
        single list of events.

        \param[out] dest The destination event list to which all the
        events are to added.

        \param[in,out] srcBkt The bucket from where the events are to
        be moved.  After this call the srcBkt will not have any events
        in it.
    */
    static void push_back(BktEventList& dest, TwoTierBucket&& srcBkt);
    
    /** Obtain the count of events which includes the events in sub-buckets.

        \return The sum of events in all of the sub-buckets.
    */
    size_t size() const {
        ASSERT( getEventCount() == count );
        return count;
    }

    /** Convenience method to determine if the bucket is empty.

        \return This method returns true if this bucket does not
        contain any events in it.
     */
    bool empty() const {
        ASSERT( getEventCount() == count );
        return (count == 0);
    }

    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method removes computes the sub-bucket that contains all
        the events for this sender and removes events from that
        sub-bucket.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.

        \param[in,out] scans Statistics object (if stats is enabled)
        to track number of events scanned.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime
                     LQ2T_STATS(COMMA Avg& scans));
    
    /** Remove all events in this bucket for a given receiver agent
        ID.

        This is a convenience method that removes all events for a
        given receiver agent in this bucket.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.  This method has to search through all the
        sub-buckets because the condition is based on receiver (and
        not sender).

        \param[in] receiver The receier ID whose events are to be
        removed from the sub-buckets.

        \return This method returns the number of events removed.
    */
    int remove(muse::AgentID receiver);
    
    /** This method is purely for troubleshooting one scenario
        where an event would get stuck in the ladder and not get
        scheduled correctly.

        \param[in] recvTime The time to be used for checking to see if
        sub-buckets have an event before this time.

        \return Returns true if an event before this receiveTime (for
        any agent) is pending in a sub-bucket.
    */
    bool haveBefore(const Time recvTime) const;

    /** Convenience method to remove all events from a given sender
        that were sent at-or-after the given sendTime.

        This method linearly scans the given event list, checks, and
        removes all events that were sent by the sender at-or-after
        the specified send time.

        \note This method assumes unsorted list of events and does not
        preserve order of events if an event is cancelled -- this is
        because Events to be removed are moved to the back and popped
        to reduce deletion time.
        
        \param[in,out] list The list of events from where all events
        for the sender are to be removed.  This method linearly scans
        through this list.  If events are removed, the order of events
        in the list is not preserved.

        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.
        
        \return This method returns the number of events that were
        removed.
    */
    static int remove_after(BktEventList& list, muse::AgentID sender,
                            const muse::Time sendTime);

    /** Remove all events in a given event list for a given receiver
        agent ID.

        This is a convenience/helper method that removes all events
        for a given receiver agent in a given sub-bucket/list.  This
        method is used to remove events scheduled for an agent, when
        an agent is removed from the scheduler.  This method has to
        search through all the events in the list to remove them.

        \param[in] subBkt The sub-bucket or list from where events are
        to be removed.
        
        \param[in] receiver The receier ID whose events are to be
        removed from the sub-buckets.
        
        \return This method returns the number of events removed.
    */
    static int remove(BktEventList& list, muse::AgentID receiver);

    /** Obtain a reference to the list of sub-buckets in this bucket.

        \return Mutable reference to the list of sub-buckets in this
        bucket.
    */
    SubBucketList& getSubBuckets() { return subBuckets; }
    
protected:
    /** Return sum of events in each sub-bucket.

        This method is used purely for validation/debugging.  This
        method iterates over each sub-bucket in the list and returns
        the actual count of events.  This value must be consistent
        with the value in the count instance variable.

        \return The actual sum of events in various sub-buckets.
    */
    size_t getEventCount() const;

    /** Clear out all the events in this bucket.

        This method clears out all the events in various sub-buckets
        in this 2-tier bucket.  It also sets count to zero.
        
        \note This method decreases references on any pending events.
    */
    void clear();
    
private:
    /** The list of tier-2 sub-buckets that contain events distributed
        based on the hash of the receiver's ID.
    */
    SubBucketList subBuckets;
    
    /** The total number of events in all of the sub-buckets.  This
        information is primary used to quickly respond to the size()
        method calls
    */
    size_t count;
};

/** The class that forms the Top rung of a 2-tier ladder queue.

    The top-rung of the 2-tier ladder queue behaves similar to the
    ladder queue with respect to managing time stamps.  However, the
    organization is different -- events are not stored in a linear
    list.  Instead they are stored in sub-buckets based on a hash of
    the sender agent's ID.

    \note Do not call push_back directly.  Instead use the add method
    in this class to add events.
*/
class TwoTierTop : public TwoTierBucket {
    friend class TwoTierRung;
    friend class TwoTierLadderQueue;
public:
    /** Construct and initialize top to empty state.

        The constructor uses a convenience method in this class to
        reset the timestamps to zero.
    */
    TwoTierTop() {
        reset();
    }

    /** The destructor

        Currently the destructor has nothing much to do as the base
        class does all of the necessary clean-ups.
     */
    ~TwoTierTop() {}

    /** Method to add events to top and update current minimum and
        maximum time stamp values.

        \param[in] event The event to be added to the top.  This
        pointer cannot be NULL.
    */
    void add(muse::Event* event);

    /** Return the current start-time for top.

        \note This value changes when events are added/removed. So
        don't think about caching this value.
        
        \return The current start time. This value is used for
        scheduling events and creating rungs.
    */
    Time getStartTime() const { return topStart; }

    /** Returns the minimum timestamp of events in this rung.

        \note This value changes when events are added/removed. So
        don't think about caching this value.
        
        \return The minimum timestamp of events in this rung.
    */
    Time getMinTime() const { return minTS; }

    /** Returns the maximum timestamp of events in this rung.

        \note This value changes when events are added/removed. So
        don't think about caching this value.
        
        \return The maximum event timestamp in this rung.
    */
    Time getMaxTime() const { return maxTS; }

    /** Convenience method to determine if given time is within the
        <i>current</i> minmum and maximum time.

        \param[in] ts The timestamp value to be checked.
        
        \return This method returns true if getMinTime() <= ts <=
        getMaxTime().  Otherwise it returns false.
     */
    bool contains(const Time ts) const {
        return (ts >= minTS) && (ts <= maxTS);
    }

    /** Convenience method compute the bucket size for the top-level
        rung of the TwoTierLadder queue.

        \return The suggested bucket width (in terms of time) for the
        top-level rung of the TwoTierLadder.
    */
    double getBucketWidth() const {
        DEBUG(std::cout << "minTS=" << minTS << ", maxTS=" << maxTS
              << ", size=" << size() << std::endl);
        return (maxTS - minTS + size() - 1.0) / size();
    }

protected:
    /** Helper method to reset top either during construction or
        whenever it is emptied to move events into the ladder.

        \param[in] topStart An optional start time for the top rung.
    */
    void reset(const Time topStart = 0);

private:
    /** Instsance variable to track the current minimum timestamp of
        events in top.  This value changes each time a new event is
        added to the top via the add emthod.
    */
    muse::Time minTS;

    /** Instsance variable to track the current maximum timestamp of
        events in top.  This value changes each time a new event is
        added to the top via the add emthod.
    */    
    muse::Time maxTS;

    /** Instsance variable to track the last time top was reset.  This
        is used for debugging/troubleshooting purposes.
    */    
    muse::Time topStart;
};

/** The bottom most rung of the TwoTierLadder queue.  The bottom rung
    is the same as that of the standard ladder queue.  However, in
    2-tier ladder queue, the size of the bottom has been relaxed.  So
    bottom can be pretty long.  This implies that 2-tier ladder queue
    will not be O(1).  It will be O(n log n).  However, it should
    perform just fine as the ladder queue.

    \note In MUSE we have an API requirement/guarantee that all the
    concurrent events we have will be scheduled simultaneously.  This
    eases agent development in many applications.  Consequently, it is
    imperative that bottom be allowed to be long to contain all
    concurrent events.
*/
class TwoTierBottom : public BktEventList {
public:
    /** The default and only constructor.  It does not have any
        special work to do as the base class handles most of the
        tasks.
    */
    TwoTierBottom() {}

    /** Add events from a TwoTierBucket into the bottom.

        This method is used to bulk move events from a rung of the
        ladder (or top) into the bottom.  The events are added and
        sorted in preparation for scheduling.

        \param bucket The 2-tier bucket from where events are to be
        moved into the bottom rung.
    */
    void enqueue(TwoTierBucket&& bucket);

    /** Add a single event to the bottom rung.

        This method uses binary-search (O(log n)) to insert an event
        into the bottom.
        
        \param[in] event The event to be added to the bottom rung.
        This pointer cannot be NULL.  No operations are done on the
        reference-counters in this method.
    */
    void enqueue(muse::Event* event);

    /** Convenience method to dequeue events after a given time.

        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.        
    */
    int remove_after(muse::AgentID sender, const Time sendTime);
    
    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver) {
        // Use static convenience method do to this task.
        return TwoTierBucket::remove(*this, receiver);        
    }

    /** Convenience method used to dequee the next set of events for
        scheduling.

        This method is used to provide necessary implemntation to
        interface with the MUSE scheduler.  This method dequeues the
        next batch of the concurrent events for processing by a given
        agent.

        \param[out] events The container to which all the events to be
        processed is to be added.
    */
    void dequeueNextAgentEvents(muse::EventContainer& events);

    /** Convenience method for debugging/troubleshooting.

        \return The highest timestamp from the events in the bottom.
        If no events are present this method returns TIME_INFINITY.
    */
    muse::Time maxTime() const {
        // purely for debugging
        return (!empty() ? back()->getReceiveTime() : TIME_INFINITY);
    }

    /** Convenience method for debugging/troubleshooting.

        \return The minimum timestamp from the events in the bottom.
        If no events are present this method returns TIME_INFINITY.
    */
    muse::Time findMinTime() const {
        // purely for debugging
        return (!empty() ? front()->getReceiveTime() : TIME_INFINITY);
    }

    /** Convenience method to check if the entries in the bottom are
        sorted correctly.  This method is purely used for
        troubleshooting/debugging.
    */
    void validate() const;

    /** Event comparison function used by various structures in ladder
        queue.

        \param[in] lhs The left-hand-side event for comparison.  The
        pointer cannot be NULL.

        \param[in] lhs The right-hand-side event for comparison.  The
        pointer cannot be NULL.
        
        \return This method returns true if lhs is less than rhs.
        That is, lhs should be scheduled before rhs.
    */
    static inline bool compare(const muse::Event* const rhs,
                               const muse::Event* const lhs) {
        return ((lhs->getReceiveTime() > rhs->getReceiveTime()) ||
                ((lhs->getReceiveTime() == rhs->getReceiveTime() &&
                  (lhs->getReceiverAgentID() > rhs->getReceiverAgentID()))));
    }

    /** Convenience method to check to see if bottom has events before
        the specified receive time.

        This method is used for troubleshooting/debugging only.
        
        \param[in] recvTime The receive time for checking.

        \return Returns true if an event before this receiveTime (for
        any agent) is pending in the bottom rung.        
    */
    bool haveBefore(const Time recvTime) const {
        return (findMinTime() <= recvTime);
    }
    
protected:
    // Currently this class does not have any protected members.

private:
    // Currently this class does not have any private members
};

/** Class that represents one rung in the 2-tier ladder queue.

    The 2-tier rung uses the same strategy for receive time-based
    bucket creation as the regular ladder queue.  However, the
    organization of each bucket is different -- events are not stored
    in a linear list.  Instead they are stored in sub-buckets based on
    a hash of the sender agent's ID.
*/
class TwoTierRung {
public:
    /** The constructor to create an empty rung.

        The constructor merely initializes all the instance variables
        to default initial values to create an empty rung.
    */
    TwoTierRung() : rStartTS(TIME_INFINITY), rCurrTS(TIME_INFINITY),
                    bucketWidth(0), currBucket(0), rungEventCount(0) {
        LQ2T_STATS(maxBkts = 0);
    }

    /** A copy constructor required to pre-allocate rungs for ladder.

        \param[in] src The source rung from where events are to be
        copied.
    */
    TwoTierRung(TwoTierRung&& src) :
        rStartTS(src.rStartTS), rCurrTS(src.rCurrTS),
        bucketWidth(src.bucketWidth), currBucket(src.currBucket),
        bucketList(std::move(src.bucketList)),
        rungEventCount(src.rungEventCount) {
        LQ2T_STATS(maxBkts = src.maxBkts);
    }
    
    /** Convenience constructor to create a rung using events from the
        top rung.

        This is a delegating constructor that delegates the actual
        tasks to the overloaded constructor.

        \param[in] top The top bucket from where the events are to be
        created.
    */
    explicit TwoTierRung(TwoTierTop&& top) :
        TwoTierRung(std::move(top), top.getMinTime(),
                    std::max(MIN_BUCKET_WIDTH, top.getBucketWidth())) {
        // Reset top counters and update the values of topStart for next Epoch
        top.reset(top.getMaxTime());
    }

    /** Convenience constructor to create a rung with events from a
        given bucket.
        
        \param[in,out] bkt The bucket from where events are to be
        moved into this newly created rung.  After this operation data
        in the bucket is cleared.

        \param[in] rStart The start time for this rung.

        \param[in] bucketWidth The delta in receive time for each
        bucket in this rung.  The bucketWidth must be > 0.
    */
    TwoTierRung(TwoTierBucket&& bkt, const Time rStart,
                const double bucketWidth);

    /** Remove the next bucket in this rung for moving to another rung
        in the ladder.

        This method must be used to remove the next bucket from this
        rung.  The bucket is logically removed (or moved) out of this
        rung.

        \param[out] bktTime The simulation receive time associated
        with the bucket being moved out.
    */
    TwoTierBucket&& removeNextBucket(muse::Time& bktTime);

    /** Determine if this rung is empty.

        This is a convenience method that is used to determine if this
        rung contains any events to be processed.

        \return This method returns true if the rung does not have any
        events -- i.e., when the rung is empty.
    */
    bool empty() const { return (rungEventCount == 0); }

    /** Add an event to suitable bucket in this rung.

        This method computes a bucket index (based on equation #2 in
        LQ paper) using the formula:

        \code
        size_t bucketNum = (event->getReceiveTime() - rStartTS) / bucketWidth;
        \endcode
        
        \param[in] event The event to be added to a suitable bucket in
        this rung.
    */
    void enqueue(muse::Event* event);

    /** Obtain the start time for this rung.

        This method returns the rung starting time that was set when
        this rung was created.
        
        \return The starting time of this rung that determines the
        lowest timestamp event that can be added to this rung.
    */
    muse::Time getStartTime() const { return rStartTS; }

    /** Obtain the bucket width (i.e., difference in receive times for
        adjacent buckets) for this rung.

        This method returns the bucket width that was set when this
        rung was created.
        
        \return The bucket with for this rung.
    */
    double getBucketWidth() const { return bucketWidth; }

    /** The current bucket value in this ladder queue.

        The current minimum time of events that can be added to this
        rung of the ladder eueue.

        \return The minimum timestamp of events that can be added to
        the rung of this ladder queue.
    */
    muse::Time getCurrTime() const {
        return rCurrTS;
    }

    /** The maximum receive time value of event that can be added to
        this rung.

        \return The maximum receive time of an event that can be added
        to a bucket in this rung.
    */
    muse::Time getMaxRungTime() const {
        return rStartTS + (bucketList.size() * bucketWidth);
    }

    /** Convenience method to determine if a given event can be added
        to this rung.

        \param[in] event The event whose receive time is to be used to
        check to see if it can be added to this ladder.

        \return Returns true if the event can be added to this rung.
        Otherwise it returns false.
    */
    bool canContain(muse::Event* event) const;

    /** Remove all events from the given sender sent at-or-after the
        specified send time from all buckets in this rung.

        This method linearly scans the buckets, checks, and removes
        all events that were sent by the sender at-or-after the
        specified send time.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.

        \param[out] ceScanRung The stats object to be updated with
        number of events scanned in the buckets in this rung.
        
        \return This method returns the total number of events that
        were removed from this rung.
    */
    int remove_after(muse::AgentID sender, const Time sendTime
                     LQ2T_STATS(COMMA Avg& ceScanRung));

    /** Remove all events for a given receiver agent in this rung.

        This is a convenience method that removes all events for a
        given receiver agent in this rung.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.

        \param[in] receiver The receiving agent ID whose events are to
        be removed from all the buckets in this rugn.

        \param[out] ceScanRung The stats object to be updated with
        number of events scanned in the buckets in this rung.
    */
    int remove(muse::AgentID receiver
               LQ2T_STATS(COMMA Avg& ceScanRung));

    /** Check to ensure that the number of events in various buckets
        matches the count instance variable.

        This method is used only for troubleshooting/debugging
        purposes.  If counts don't match then assert fails in this
        method causing the simulation to abort.
    */
    void validateEventCounts() const;

    /** Print a user-friendly version of the events in this queue.

        Currently this method is not implemented.
    */
    void prettyPrint(std::ostream& os) const;

    /** Update the statistics object with data from this rung.

        \param[out] avgBktCnt Update the average number of buckets in
        this rung.
    */
    void updateStats(Avg& avgBktCnt) const;

    /** Convenience method to determine if the current bucket in this
        rung is empty.

        \return This method returns true if the current bucket in this
        rung is empty.
    */
    bool isCurrBucketEmpty() const {
        return (currBucket >= bucketList.size() ||
                bucketList[currBucket].empty());
    }

    /** This method is purely for troubleshooting one scenario where
        an event would get stuck in the ladder and not get scheduled
        correctly.
        
        \param[in] recvTime The time to be used for checking to see if
        sub-buckets have an event before this time.
        
        \return Returns true if an event before this receiveTime (for
        any agent) is pending in a sub-bucket.
    */    
    bool haveBefore(const Time recvTime) const;
    
protected:
    // Currently this class does not have any protected members.
private:
    /** The lowest timestamp event that can be added to this rung.
        This value is set when a rung is created and is never changed
        during the lifetime of this rung.
    */
    muse::Time rStartTS;

    /** The timestamp of the lowest event that can be currently added
        to this rung.  This value logically starts with rStartTS and
        grows to the time stamp of last bucket in this rung as buckets
        are dequeued from this rung.
    */
    muse::Time rCurrTS;

    /** The width of the bucket in simulation receive time
        differences.  This value can be fractional.
    */
    double bucketWidth;
    
    /** The index of the current bucket on this rung to which events
        can be added.  This is also the next bucket that will be
        dequeued from the rung.
    */
    size_t currBucket;

    /** The deque containing the set of vectors in this bucket list.
     */
    std::deque<TwoTierBucket> bucketList;
    
    /** Total number of events still present in this rung.  This is
        used to report size and check for empty quickly.
    */
    int rungEventCount;

    /** Statistics object to track the maximum number of buckets used
        in this rung */
    LQ2T_STATS(size_t maxBkts);
};

/** The top-level 2-tier ladder queue

    <p>This class represents the top-level 2-tier ladder queue class
    that interfaces with the MUSE scheduler.  This class implements
    the top-level logic associated with ladder queue to enqueue,
    dequeue, and cancel events from the ladder queue.</p>

    <p>The logic for most of the operations is consistent with those
    proposed by the Tang et. al, except for the following:

    <ol>

    <li>The size of the bottom is not restricted.  So events are never
    moved from bottom back into the ladder.</li>

    <li> The number of buckets in a rung is restricted to 100</li>

    </ol>

    </p>
*/
class TwoTierLadderQueue : public EventQueue {
public:
    /** The constructor that creates an empty ladder queue.

        The constructor also initializes various statistics variables
        used by the this queue to report detailed statistics about its
        operations at the end of simulation.
     */
    TwoTierLadderQueue() : EventQueue("LadderQueue"), ladderEventCount(0) {
        ladder.reserve(MAX_RUNGS);
        LQ2T_STATS(ceTop    = ceLadder  = ceBot  = 0);
        LQ2T_STATS(insTop   = insLadder = insBot = 0);
        LQ2T_STATS(maxRungs = 0);
    }
    
    /** The destructor.

        Currently the destructor does not have anything special to do
        as the different encapsulated objects handle all the necessary
        clean-up.
    */
    ~TwoTierLadderQueue() {}

    /** Enqueue an event into the laadder queue.

        Depending on the scenario the event is appropriately added to
        one of: top, ladder rung, or the bottom.
        
        \param[in] e The event to be enqueued for scheduling in the
        ladder queue.
    */
    void enqueue(muse::Event* e);

    /** Cancel all events from a given sender that were sent
        at-or-after the specified send time.

        This method essentially calls the corresponding method(s) in
        top, rung, and bottom to cancel pending events.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.

        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Determine if the ladder queue is empty.

        Implements the interface method used by MUSE::Scheduler.
        
        \return Returns true if top, ladder, and bottom are all empty
        -- i.e., there are no pending events.
    */
    virtual bool empty() {
        return top.empty() && (ladderEventCount == 0) &&  bottom.empty();
    }

    /** Implementation for method used by MUSE::Scheduler.

        This method is called by MUSE kernel to inform the scheduler
        queue about an agent being added during initialization.  The
        ladder queue does not utilize this information and
        consequently this method does not have any special operation
        to perform.

        \param[in] agent The agent being added.  This pointer is not
        really used.
        
        \return This method simply returns nullptr as the ladder queue
        does not use any cross references in muse::Agent for its
        operations.
    */
    virtual void* addAgent(muse::Agent* agent);

    /** Remove an agent just before simulation completes.

        This method is invoked by the MUSE kernel to inform that an
        agent is being removed.  This method removes all pending
        events for the specified agent from the ladder queue.

        \param[in] agent The agent whose sender ID is used to remove
        all pending events in the top, rungs, and bottom.
    */
    virtual void removeAgent(muse::Agent* agent);

    /** Implement interface method to peek at the next event to
        schedule.

        \note In order to enable peeking of the front event, the
        bottom may need to get populated.
        
        \return A pointer to the next event to schedule (if any).  The
        event is not dequeued.
     */
    virtual muse::Event* front();

    /** This method is used to provide necessary implemntation to
        interface with the MUSE scheduler.  This method dequeues the
        next batch of the concurrent events for processing by a given
        agent.

        \param[out] events The container to which all the events to be
        processed is to be added.
    */
    virtual void dequeueNextAgentEvents(muse::EventContainer& events);

    /** Add an event to be scheduled to this ladder queue.

        This method implements the core API used by agents to schedule
        events for each other.
        
        \param[in] agent The receiver agent for which the event is
        scheduled.  This pointer is not used.

        \param[in] event The event to be scheduled. This simply calls
        the overloaded enqueue method.  The reference count on the
        event is increased by this method to account for this event
        being present in the ladder queue.
    */
    virtual void enqueue(muse::Agent* agent, muse::Event* event);

    /** Enqueue a batch of events

        This API to schedule a block of events.  This API is typically
        used after a rollback.

        \param[in] agent The receiver agent for which the event is
        scheduled.  This pointer is not used.

        \param[in] events The list of events to be scheduled. This
        simply calls the overloaded enqueue method to enqueue one
        event at a time.
    */
    virtual void enqueue(muse::Agent* agent, muse::EventContainer& events);

    /** Implement MUSE kernel API to cancel all events sent by a given
        agent after a given time.

        \param[in] dest The destination agent whose events are to be
        cancelled.

        \param[in] sender The sender agent ID whose events are to be
        cancelled.

        \param[in] sentTime The send time at-or-after which all events
        from the sender are to be cancelled.
    */
    virtual int eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                           const muse::Time sentTime);

    /** Print a human understandable version of the events in this
        queue.

        currently this method is not implemented.
     */
    virtual void prettyPrint(std::ostream& os) const;

    
    /** Convenience method to check to see if ladder queue has events
        before the specified receive time.

        This method is used for troubleshooting/debugging only.
        
        \param[in] recvTime The receive time for checking.

        \return Returns true if an event before this receiveTime (for
        any agent) is pending in the bottom rung.        
    */    
    bool haveBefore(const Time recvTime,
                    const bool checkBottom = false) const;
    
    /** Method to report aggregate statistics.

        This method is invoked at the end of simulation after all
        agents on this rank have been finalized.  This method is meant
        to report any aggregate statistics from this queue.  This
        method writes statistics only if LQ2T_STATS macro is enabled.
        
        \param[out] os The output stream to which the statistics are
        to be written.
    */
    virtual void reportStats(std::ostream& os);
    
protected:
    /** Check and create rungs in the ladder and return the next
        bucket of events from the ladder.

        This method implements the corresponding recurseRung method
        from the LQ paper. Refer to the paper for the details.
    */
    TwoTierBucket&& recurseRung();

    void populateBottom();
    
private:
    TwoTierTop top;
    std::vector<TwoTierRung> ladder;
    int ladderEventCount;
    TwoTierBottom bottom;

    LQ2T_STATS(Avg ceTop);
    LQ2T_STATS(Avg ceBot);
    LQ2T_STATS(Avg ceLadder);

    LQ2T_STATS(Avg ceScanTop);
    LQ2T_STATS(Avg ceScanLadder);

    /** The ceScanBot statistic tracks size of bottom rung scanned
        when at one (or more) events were canceled from bottom.
    */
    LQ2T_STATS(Avg ceScanBot);
    
    /** The ceNoCanScanBot statistic tracks size of bottom rung
        scanned but did not cancel any events.
    */
    LQ2T_STATS(Avg ceNoCanScanBot);
    
    LQ2T_STATS(int insTop);
    LQ2T_STATS(int insLadder);
    LQ2T_STATS(int insBot);
    LQ2T_STATS(size_t maxRungs);
    LQ2T_STATS(Avg avgBktCnt);
    LQ2T_STATS(Avg botLen);
    LQ2T_STATS(Avg avgBktWidth);
};

END_NAMESPACE(muse)

#endif
