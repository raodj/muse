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

/** \def LQ2T_STATS(x)

    \brief Define a convenient macro for conditionally compiling
    additional statistics collection regarding ladder queue.

    Define a custom macro LQ2T_STATS (note the all caps) macro to be
    used to conditionally compile in debugging code to generate
    detailed logs.  This helps to minimize code modification to insert
    and remove debugging messages.
*/
#define COMMA ,
#define LQ_STATS(x) x
// #define LQ_STATS(x)

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
    TwoTierBucket() : count(0) {}

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

        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);
    
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
    
    /** The method below is purely for troubleshooting one scenario
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
        the specified send time.  It is assumed that the list is not
        sorted and does not need to be preserved.  Events to be
        removed are moved to the back and popped to reduce deletion
        time.
        
        \param[in,out] list The list of events from where all events
        for the sender are to be removed.  This method linearly scans
        through this list.  If events are removed, the contents of the
        list is not preserved.

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
    int remove_after(muse::AgentID sender, const Time sendTime) {
        // Use static convenience method do to this task.
        return TwoTierBucket::remove_after(*this, sender, sendTime);
    }
    
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
    muse::Time maxTime() const;  // purely for debugging

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
        
        \return This method returns true if lhs is greater than rhs.
        That is, lhs should be scheduled after rhs.
    */
    static inline bool compare(const muse::Event* const lhs,
                               const muse::Event* const rhs) {
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

END_NAMESPACE(muse)

#endif
