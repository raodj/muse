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
    /** Constructor to create a bucket with fixed number (i.e., t2k)
        of tier-2 lists.
        
        \param[in] t2k The number tier-2 sub-buckets to create.
    */
    TwoTierBucket(const int t2k) : t2k(t2k), count(0) {}

    /** A move constructor to facilitate moving objects (if needed).

        \param[in,out] src The source object whose data is to be moved
        into this.  The source object does not contain any useful
        information after the move is complete.
    */
    TwoTierBucket(TwoTierBucket&& src) : subBuckets(std::move(src.subBuckets)),
                                         t2k(src.t2k),
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

        \param[in] t2k The maximum number of sub-buckets to choose
        from.

        \return The hash value based on sender ID.  The return value,
        say hash, must be in the range 0 <= hash < t2k.
    */
    inline int hash(const muse::AgentID sender, const int t2k) const {
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
        const size_t subBktIdx = hash(event->getSenderAgentID(), t2k);
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
    int remove_after(muse::AgentID sender, const Time sendTime) {
        const size_t subBktIdx = hash(sender, t2k);
        return remove_after(subBuckets[subBktIdx], sender, sendTime);
    }
    
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

protected:
    /** Return sum of events in each sub-bucket.

        This method is used purely for validation/debugging.  This
        method iterates over each sub-bucket in the list and returns
        the actual count of events.  This value must be consistent
        with the value in the count instance variable.

        \return The actual sum of events in various sub-buckets.
    */
    size_t getEventCount() const;

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
    int remove_after(BktEventList& list, muse::AgentID sender,
                     const muse::Time sendTime);
    
private:
    /** The list of tier-2 sub-buckets that contain events distributed
        based on the hash of the receiver's ID.
    */
    SubBucketList subBuckets;

    /** The number of sub-buckets into whcih this TwoTierBucket is to
        be organized.  This value is set once in the constructor and
        does not change during the lifetime of this object.
    */
    int t2k;

    
    /** The total number of events in all of the sub-buckets.  This
        information is primary used to quickly respond to the size()
        method calls
    */
    size_t count;
};

END_NAMESPACE(muse)

#endif
