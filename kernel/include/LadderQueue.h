#ifndef LADDER_QUEUE_H
#define LADDER_QUEUE_H

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

#include <list>
#include <queue>
#include <vector>
#include <set>
#include "Avg.h"
#include "Event.h"
#include "EventQueue.h"

/** \file LadderQueue.h

    \brief Implementation for a LadderQueue.

    The LadderQueue.h and LadderQueue.cpp collectively provide
    implementation for a LadderQueue data structure.  The data
    structure is detailedin the following paper:

    W. Tang, R. Goh, and I. Thng, "Ladder queue: An O(1) priority
    queue structure for large-scale discrete event simulation", ACM
    TOMACS, Vol 15, Issue 3, Pages 175--204, July 2005. URL: 
    http://doi.acm.org/10.1145/1103323.1103324
*/

// The threshold value (number of events in bottom) after which events
// from bottom are placed into a new rung in the ladder.
#define THRESH 50

/** \def LQ_STATS(x)

    \brief Define a convenient macro for conditionally compiling
    additional statistics collection regarding ladder queue.

    Define a custom macro LQ_STATS (note the all caps) macro to be used
    to conditionally compile in debugging code to generate detailed
    logs.  This helps to minimize code modification to insert and
    remove debugging messages.
*/
#define COMMA ,
#define LQ_STATS(x) x
// #define LQ_STATS(x)

BEGIN_NAMESPACE(muse)

// Defintion for a vector events
using EventVector = std::vector<muse::Event*>;

// The definition for a singly-linked list of Events
using EventList = std::list<muse::Event*>;

/** A generic bucket that is used for both Top and Rungs of ladderQ.

    <p>This bucket does not store events in it directly. Instead it
    provides a singly-linked list for storing Events.
    </p>
*/ 
class ListBucket {
public:
    /** Constructor to create a bucket with a singly-linked list of Events.
    */
    ListBucket() : count(0) {}
    
    /** A move constructor to facilitate moving objects (if needed).

        \param[in,out] src The source object whose data is to be moved
        into this.  The source object does not contain any useful
        information after the move is complete.
    */
    ListBucket(ListBucket&& src) : list(std::move(src.list)),
                           count(std::move(src.count)) {
        // Reset count in source to aid debugging.
        src.count = 0;
    }
    
    /** The destructor for this class.

        The destructor decreases the reference count on all the events
        in its list to free-up any pending events.
    */
    ~ListBucket();
    
    // Definition of an iterator for the event list.
    using iterator = EventList::iterator;
    // Definition of a const iterator for the event list.
    using const_iterator = EventList::const_iterator;

    /** Convenience method to add events to the event list.

        This is an internal convenience method that is used to add
        events to the front of the event list.
    */
    void push_front(muse::Event* event) {
        list.push_front(event);
        count++;
    }
    
    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in this event list.
        
        \return A pointer to the next event to be processed.  If the
        list is empty then this method returns NULL.
    */
    muse::Event* front() const {
        return (!list.empty() ? list.front() : NULL);
    }
    
    /** Obtain pointer to the last event in the list.

        This method can be used to obtain a pointer to the last event
        in this event list.
        
        \return A pointer to the last event to be processed.  If the
        list is empty then this method returns NULL.
    */
    muse::Event* back() const {
        return (!list.empty() ? list.back() : NULL);
    }
    
    /** Convenience method to remove an event from the event list.

        This is an internal convenience method that is used to remove
        the front (i.e., events with lowest timestamp) event from this
        list.
     
        \return A pointer to the highest priority event in this event list.
    */
    muse::Event* pop_front() {
        muse::Event* retVal = list.front();
        list.pop_front();
        count--;
        return retVal;
    }

    /** Convenience method to insert an event into the event list.

        This is an internal convenience method that is used to insert
        an event at a specified location in the event list.  
    */
    void insert_after(ListBucket::iterator pos, muse::Event* event) {
        list.insert(++pos, event);
        count++;
    }
    
    /** Obtain the count of events.

        \return The sum of events in the singly-linked list.
    */
    size_t size() const { return count; }
    
    /** Determine if the event list is empty.
        
        \return This method returns true if the event list of the
        bucket is logically empty.
    */    
    bool empty() const { return list.empty(); }
    
    /** Obtain the iterator to the beginning of the list.

        \return The iterator to the first element in the list.
    */
    ListBucket::iterator begin() { return list.begin(); }
    
    /** Obtain the iterator to the beginning of the list.

        \return The const iterator to the first element in the list.
    */
    ListBucket::const_iterator cbegin() { return list.cbegin(); }

    /** Obtain the iterator to the end of the list.

        \return The iterator to the last element in the list.
    */
    ListBucket::iterator end() { return list.end(); }
    
    /** Obtain the iterator to the end of the list.

        \return The const iterator to the last element in the list.
    */
    ListBucket::const_iterator cend() { return list.cend(); }
    
    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);
    
    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the list.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after_sorted(muse::AgentID sender, const Time sendTime) {
        // No difference between sorted and unsorted version for ListBucket
        return remove_after(sender, sendTime);
    }

    /** Remove all events for a given receiver agent ID.

        This is a convenience method that removes all events for a
        given receiver agent.  This method is used to remove events
        scheduled for an agent, when an agent is removed from the
        scheduler.
    */
    int remove(muse::AgentID receiver);
    
    // The method below is purely for troubleshooting one scenario
    // where an event would get stuck in the ladder and not get
    // scheduled correctly.
    bool haveBefore(const Time recvTime) const;
    
private:
    
    /** The singly-linked list of events.
    */
    EventList list;
    
    /** The total number of events in the singly-linked list of events. This
        information is primary used to quickly respond to the size()
        method calls.
    */
    size_t count;
};

/** A generic bucket that is used for both Top and Rungs of ladderQ.

    <p>This bucket does not store events in it directly. Instead it
    provides a vector for storing Events.
    </p>
*/
class VectorBucket {
public:
    /** Constructor to create a bucket with a vector container for holding
        Events.
    */
    VectorBucket() : count(0) {}
    
    /** A move constructor to facilitate moving objects (if needed).

        \param[in,out] src The source object whose data is to be moved
        into this.  The source object does not contain any useful
        information after the move is complete.
    */
    VectorBucket(VectorBucket&& src) : list(std::move(src.list)),
                                       count(std::move(src.count)) {
        // Reset count in source to aid debugging.
        src.count = 0;
    }
    
    /** The destructor for this class.

        The destructor decreases the reference count on all the events
        in the vector of events to free-up any pending events.
    */
    ~VectorBucket();
    
    // Definition of an iterator for the vector containing events.
    using iterator = EventVector::iterator;
    // Definition of a const iterator for the vector containing events.
    using const_iterator = EventVector::const_iterator;
    // Definition of a reverse iterator for the vector containing events.
    using reverse_iterator = EventVector::reverse_iterator;

    /** Convenience method to add events to the vector of events.

        This is an internal convenience method that is used to add
        events to the vector of events.
    */
    void push_front(muse::Event* event) {
        list.push_back(event);
        count++;
    }

    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in the vector of events.
        
        \return A pointer to the next event to be processed.  If the
        vector is empty then this method returns NULL.
    */
    muse::Event* front() const {
        return (!list.empty() ? list.back() : NULL);
    }

    /** Obtain pointer to the last event in the vector .

        This method can be used to obtain a pointer to the last event
        in this vector of events.
        
        \return A pointer to the last event to be processed.  If the
        vector is empty then this method returns NULL.
    */
    muse::Event* back() const {
        return (!list.empty() ? list.front() : NULL);
    }
    
    /** Convenience method to remove an event from the vector of events.

        This is an internal convenience method that is used to remove
        the front (i.e., events with lowest timestamp) event from the
        vector of events.
     
        \return A pointer to the highest priority event in the vector of events.
    */
    muse::Event* pop_front() {
        muse::Event* retVal = list.back();
        list.pop_back();
        count--;
        return retVal;
    }

    /** Convenience method to add events to the vector of events.

        This is an internal convenience method that is used to add
        events from a bucket to the vector of events.
    */
    void push_back(VectorBucket&& bucket);
    
    /** Convenience method to insert an event into the vector of events.

        This is an internal convenience method that is used to insert
        an event at a specified location in the vector of events.  
    */
    void insert_after(VectorBucket::iterator pos, muse::Event* event) {
        list.insert(pos + 1, event);
        count++;
    }

    /** Convenience method to insert an event into the vector of events.

        This is an internal convenience method that is used to insert
        an event at a specified location in the vector of events.  
    */
    void insert_after(VectorBucket::reverse_iterator pos, muse::Event* event) {
        const size_t idx = list.size() - (pos - rbegin());
        list.insert(list.begin() + idx, event);
        count++;
    }
    
    /** Obtain the count of events.

        \return The sum of events in the vector of events.
    */
    size_t size() const { return count; }

    /** Determine if the vector of events is empty.
        
        \return This method returns true if the vector of events of the
        bucket is logically empty.
    */
    bool empty() const { return list.empty(); }

    /** Obtain the iterator to the beginning of the vector of events.

        \return The iterator to the first element in the vector of events.
    */
    VectorBucket::iterator begin() { return list.begin(); }
    
    /** Obtain the iterator to the beginning of the vector of events.

        \return The iterator to the first element in the vector of events.
    */
    VectorBucket::const_iterator cbegin() const { return list.cbegin(); }
    
    /** Obtain the iterator to the beginning of the vector of events.

        \return The reverse iterator to the beginning element in the
         vector of events.
    */
    VectorBucket::reverse_iterator rbegin() { return list.rbegin(); }
    
    /** Obtain the iterator to the end of the vector of events.

        \return The iterator to the last element in the vector of events.
    */
    VectorBucket::iterator end() { return list.end(); }
    
    /** Obtain the iterator to the end of the vector of events.

        \return The const iterator to the last element in the vector of events.
    */
    VectorBucket::const_iterator cend() const { return list.cend(); }
    
    /** Obtain the iterator to the end of the vector of events.

        \return The reverse iterator to the last element in the vector of events.
    */
    VectorBucket::reverse_iterator rend() { return list.rend(); }

    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the vector.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
     
        \note This method assumes a sorted vector of events and shortcircuit
        scans the vector if last event's time is less-or-equal to sendTime. 
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the given list.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after_sorted(muse::AgentID sender, const Time sendTime);
            
    /** Remove all events in this vector bucket for a given receiver
        agent ID.

        This is a convenience method that removes all events for a
        given receiver agent in this bucket.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);
    
    // The method below is purely for troubleshooting one scenario
    // where an event would get stuck in the ladder and not get
    // scheduled correctly.
    bool haveBefore(const Time recvTime) const;

private:
    /** The vector of events.
     */
    EventVector list;
    
    /** The total number of events in the vector of events. This
        information is primary used to quickly respond to the size()
        method calls.
    */
    size_t count;
};

// using Bucket = ListBucket;
using Bucket = VectorBucket;

/** The class that forms the Top rung of a ladder queue.

    The top-rung of the ladder queue stores events in a VectorBucket that is
    backed by a vector.

    \note Do not call push_back directly. Instead use the add method
    in this class to add events.
*/
class Top {
    friend class Rung;
    friend class LadderQueue;
public:
    /** Construct and initialize top to empty state.

        The constructor uses a convenience method in this class to
        reset the timestamps to zero.
    */
    Top() { reset(); }
    
    /** The destructor

        Currently the destructor has nothing to do.
     */
    ~Top();
    
    /** Convenience method to add events to the top-rung.

        This is an internal convenience method that is used to add
        events to the top-rung by calling push_front method of VectorBucket
        and adding an event to the bucket's vector of events.
    */
    void add(muse::Event* event);
    
    /** Determine if the top-rung is empty.
        
        \return This method returns true if the bucket of events is 
        logically empty.
    */
    bool empty() const { return events.empty(); }
    
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
        
        \return This method returns true if minTS <= ts <= maxTS.
        Otherwise it returns false.
    */
    bool contains(const Time ts) const {
        return (ts >= minTS) && (ts <= maxTS);
    }
    
    /** Convenience method compute the bucket size for the top-level
        rung of the Ladder queue.

        \return The suggested bucket width (in terms of time) for the
        top-level rung of the Ladder queue.
    */
    double getBucketWidth() const {
        DEBUG(std::cout << "minTS=" << minTS << ", maxTS=" << maxTS
                        << ", size=" << size() << std::endl);
        return std::max((maxTS - minTS + size() - 1.0) / size(), 0.01);
    }
    
    /** Obtain the count of events.

        \return The sum of events in the top-rung of the Ladder queue.
    */
    int size() const { return events.size(); }

    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the vector.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);
    
    // The method below is purely for troubleshooting one scenario
    // where an event would get stuck in the ladder and not get
    // scheduled correctly.
    bool haveBefore(const Time recvTime) const {
        return events.haveBefore(recvTime);
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
    
    /** Instance variable to track the current maximum timestamp of
        events in top.  This value changes each time a new event is
        added to the top via the add method.
    */  
    muse::Time maxTS;
    
    /** Instance variable to track the last time top was reset.  This
        is used for debugging/troubleshooting purposes.
    */
    muse::Time topStart;
    
    /**The VectorBucket backed by a vector that stores events.
     */
    Bucket events;
};

/** The bottom most rung of the Ladder queue.  The bottom rung
    is the same as that of the standard ladder queue.  However, in this
    implementation of ladder queue, the size of the bottom has been relaxed.  
    So bottom can be pretty long.  This implies ladder queue will not be O(1).
    It will be O(n log n). However, it should perform just fine as the ladder
    queue.

    \note In MUSE we have an API requirement/guarantee that all the
    concurrent events we have will be scheduled simultaneously.  This
    eases agent development in many applications. Consequently, it is
    imperative that bottom be allowed to be long to contain all
    concurrent events.
 
*/
class Bottom {
    friend class LadderQueue;   // NOTE: uses sel directly
public:
    
    /** Add events from a Bucket (VectorBucket or ListBucket) into the bottom.

        This method is used to bulk move events from a rung of the
        ladder (or top) into the bottom.  The events are added and
        sorted in preparation for scheduling.

        \param bucket The bucket from where events are to be
        moved into the bottom rung.
    */
    void enqueue(Bucket&& bucket) {
        // Delegate to overloaded method to handle different scenarios
        // depending on whether linked-list or vector is used for Buckets
        enqueue(std::move(bucket), sel);
    }
    
    /** Add a single event into the bottom.

        This method is used to add a single event from a rung of the
        ladder (or top) into the bottom. The event is added into a sorted list
        or vector of events in preparation for scheduling.

        \param Event The event to be added into the bottom rung.
    */
    void enqueue(muse::Event* event) {
        // Delegate to overloaded method to handle different scenarios
        // depending on whether linked-list or vector is used for Buckets
        enqueue(event, sel);        
    }
    
    /** Convenience method to remove an event from the vector or list of events.

        This is an internal convenience method that is used to remove
        the front (i.e., events with lowest timestamp) event 
        depending on whether linked-list or vector is used for Buckets
     
        \return A pointer to the highest priority event in the vector of events.
    */
    muse::Event* pop_front() { return sel.pop_front(); }
    
    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in the vector or list of events.
        
        \return A pointer to the next event to be processed.  If the
        vector is empty then this method returns NULL.
    */
    muse::Event* front() const { return sel.front(); }

    /** Determines if Bottom is empty.
        
        \return This method returns true if the bucket of events is
        logically empty.
    */
    bool empty() const {
        return sel.empty();
    }

    /** Determine bucket width to move bottom into ladder.

        This method is invoked only when the ladder is empty and the
        bottom is long and needs to be moved into the ladder.  This
        method must compute and return the preferred bucket width.

        \note If the bottom is empty this method returns bucket width
        of 0.
     */
    double getBucketWidth() const;
    
    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \note When a vector is used for bucket, this method assumes a sorted
        vector of events and shortcircuit scans the vector if last event's
        time is less-or-equal to sendTime.
     
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the vector.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime) {
        return sel.remove_after_sorted(sender, sendTime);
    }
    
    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);
    
    /** Convenience method used to dequeue the next set of events for
        scheduling.

        This method is used to provide necessary implementation to
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
    muse::Time maxTime() const;  
    
    /** Convenience method for debugging/troubleshooting.

        \return The minimum timestamp from the events in the bottom.
        If no events are present this method returns TIME_INFINITY.
    */
    muse::Time findMinTime() const; 
    
    /** Convenience method to check if the entries in the bottom are
        sorted correctly. This method is purely used for
        troubleshooting/debugging.
    */
    void validate();
    
    /** Obtain the count of events.

        \return The sum of events in the bottom of the Ladder queue.
    */
    inline size_t size() const { return sel.size(); }

    /** Event comparison function used by various structures in ladder
        queue.

        \return This method returns true if lhs is greater than rhs.
        That is, lhs should be scheduled after rhs.
     */
    static inline bool compare(const muse::Event* const lhs,
                               const muse::Event* const rhs) {
        return ((lhs->getReceiveTime() > rhs->getReceiveTime()) ||
                ((lhs->getReceiveTime() == rhs->getReceiveTime() &&
                  (lhs->getReceiverAgentID() > rhs->getReceiverAgentID()))));
    }

    /** Event comparison function used by various structures in ladder
        queue. This comparison reverses the sort order -- it places
        lowest timestamp towards end of the vector.  This make it more
        efficient for popping element off the end of the vector using
        the pop_front() method in this class.

        \return This method returns true if lhs is greater than rhs.
        That is, lhs should be scheduled after rhs.
     */
    static inline bool revCompare(const muse::Event* const lhs,
                               const muse::Event* const rhs) {
        return compare(rhs, lhs);
    }
    
    // The method below is purely for troubleshooting one scenario
    // where an event would get stuck in the ladder and not get
    // scheduled correctly.
    bool haveBefore(const Time recvTime) const {
        return sel.haveBefore(recvTime);
    }

    /** Method to determine the range of receive time values currently
        in bottom.  This value is used to decide if it is worth moving
        events from bottom into the ladder.

        \return The difference in maximum and minimum receive
        timestamp of events in the bottom.  This value is zero if all
        events have the same receive time.  If the bottom is empty,
        then this method also returns zero.
    */
    muse::Time getTimeRange() const {
        if (sel.empty()) {
            return 0;
        }
        return (sel.back()->getReceiveTime() - sel.front()->getReceiveTime());
    }
	
protected:
    // Two different strategies based on the type of bucket used -->
    // linked-list vs. vector
    void enqueue(ListBucket&& bucket, ListBucket& botList);
    void enqueue(VectorBucket&& bucket, VectorBucket& botList);

    // Two different strategies based on the type of bucket used -->
    // linked-list vs. vector
    void enqueue(muse::Event* event, ListBucket& botList);
    void enqueue(muse::Event* event, VectorBucket& botList);
    
private:
    // Sorted Event List (SEL) for the bottom
    Bucket sel;
};

/** An alternative implementation for Bottom that uses a binary heap.
    The objective of having multiple implementations for Bottom is to identify
    the best data structure for the type of operations that are performed on
    Bottom in both sequential and TimeWarp simulations.
*/
class HeapBottom {
    friend class LadderQueue;   // NOTE: uses sel directly
public:
    
    /** Constructor to create a heap based Bottom for Ladder Queue.

        The constructor initializes the max event time to zero.
    */
    HeapBottom() : maxEvtTime(0) {}
    
    /** Add events from a Bucket into the heap bottom.

        This method is used to bulk move events from a rung of the
        ladder (or top) into the bottom.  The events are added and
        place in a vector backed heap in preparation for scheduling.

        \param bucket The bucket from where events are to be
        moved into the bottom rung.
    */
    void enqueue(Bucket&& bucket);
    
    /** Add a single event into the heap bottom.

        This method is used to add a single event into the bottom.
        The event is added into a vector backed heap in 
        preparation for scheduling.

        \param Event The event to be added into the bottom rung.
    */
    void enqueue(muse::Event* event);
    
    /** Convenience method to remove an event from the vector of events.

        This is an internal convenience method that is used to remove
        the front (i.e., event with lowest timestamp) event.
     
        \return A pointer to the highest priority event in the vector of events.
    */
    muse::Event* pop_front();
    
    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in the vector of events.
        
        \return A pointer to the next event to be processed. 
    */
    muse::Event* front() const { return sel.front(); }

    /** Determines if HeapBottom is empty.
        
        \return This method returns true if the vector of events is
        logically empty.
    */
    bool empty() const { return sel.empty();  }

    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the vector.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);

    /** Convenience method used to dequeue the next set of events for
        scheduling.

        This method is used to provide necessary implementation to
        interface with the MUSE scheduler.  This method dequeues the
        next batch of the concurrent events for processing by a given
        agent.

        \param[out] events The container to which all the events to be
        processed is to be added.
    */
    void dequeueNextAgentEvents(muse::EventContainer& events);
    
    /** Convenience method for debugging/troubleshooting.

        \return The highest timestamp from the events in the heap-bottom.
        If no events are present this method returns TIME_INFINITY.
    */ 
    muse::Time maxTime() const; 
    
    /** Convenience method for debugging/troubleshooting.

        \return The minimum timestamp from the events in the heap-bottom.
        If no events are present this method returns TIME_INFINITY.
    */
    muse::Time findMinTime() const; 
    
    /** Convenience method to check if the entries in the bottom are
        sorted correctly. This method is purely used for
        troubleshooting/debugging.
    */
    void validate();

    /** Obtain the count of events.

        \return The sum of events in the heap-bottom of the Ladder queue.
    */
    inline size_t size() const { return sel.size(); }

    // The method below is purely for troubleshooting one scenario
    // where an event would get stuck in the ladder and not get
    // scheduled correctly.
    bool haveBefore(const Time recvTime) const;

    /** Method to determine the range of receive time values currently
        in bottom.  This value is used to decide if it is worth moving
        events from bottom into the ladder.

        \note This method is called often, particularly in parallel
        simulation.  Consequently, it needs to be quick. To ensure it
        is quick, we track the maximum event time in the min-heap
        using the maxEvtTime instance variable.
        
        \return The difference in maximum and minimum receive
        timestamp of events in the bottom.  This value is zero if all
        events have the same receive time.  If the bottom is empty,
        then this method also returns zero.
    */
    muse::Time getTimeRange() const {
        if (sel.empty()) {
            return 0;
        }
        return (maxEvtTime - sel.front()->getReceiveTime());
    }

    /** Determine bucket width to move bottom into ladder.

        This method is invoked only when the ladder is empty and the
        bottom is long and needs to be moved into the ladder.  This
        method must compute and return the preferred bucket width.

        \note If the bottom is empty this method returns bucket width
        of 0.
     */
    double getBucketWidth() const;
    
protected:
    // Currently there are no protected members in this class
    void print(std::ostream& os = std::cout) const;
private:
    // Vector of events
    EventVector sel;
    // The maximum event time in the heap bottom.
    muse::Time maxEvtTime;
};


/** Comparator for multi-set to ensure least-time-stamp-first (LTSF)
    ordering in the set of events.  This definition is necessary
    because the Bottom::compare used with heap arranges events in
    reverse order when used with std::multiset.
*/
struct MultiSetComparator {
    inline bool operator()(const muse::Event* const lhs,
                           const muse::Event* const rhs) {
        return Bottom::compare(rhs, lhs);
    }
};

// The definition for a multi-set of Events
using EventMultiSet = std::multiset<muse::Event*, MultiSetComparator>;
                                    
/** An alternative implementation for Bottom that uses a std::multiset
    instead of a binary heap.  The objective of having multiple
    implementations for Bottom is to identify the best data structure
    for the type of operations that are performed on Bottom in both
    sequential and TimeWarp simulations.
*/
class MultiSetBottom {
    friend class LadderQueue;   // NOTE: uses sel directly
public:
    // MultiSetBottom() : sel(MultiSetComparator()) {}
    
    /** Add events from a Bucket into the multi-set bottom.

        This method is used to bulk move events from a rung of the
        ladder (or top) into the bottom.  The events are added and
        place in a multi-set bottom in preparation for scheduling.

        \param bucket The bucket from where events are to be
        moved into the bottom rung.
    */
    void enqueue(Bucket&& bucket);
    
    /** Add a single event into the Multi-set bottom.

        This method is used to add a single event into the bottom.
        The event is added into a multi-set backed Bottom in 
        preparation for scheduling.

        \param Event The event to be added into MultiSetBottom.
    */
    void enqueue(muse::Event* event);

    /** Convenience method to remove an event from the multi-set container
        of events.

        This is an internal convenience method that is used to remove
        the front (i.e., event with lowest timestamp) event.
     
        \return A pointer to the highest priority event in the multi-set
        container of events.
    */
    muse::Event* pop_front();
    
    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in the multi-set container that stores the events.
        
        \return A pointer to the next event to be processed. 
    */
    muse::Event* front() const { return *sel.cbegin(); }

    /** Determines if MultiSetBottom is empty.
        
        \return This method returns true if the multi-set container that holds 
        the events is logically empty.
    */
    bool empty() const { return sel.empty();  }

    /** Convenience method to remove all events sent by the sender
        at-or-after the given send Time.

        This method will compare the timestamps of all events in the
        list with that of the specified event. Any Event (from the
        same sender) that was sent at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] sender The sender agent whose events are to be
        removed.

        \param[in] sendTime The time at-or-after which events from the
        sender are to be removed from the multi-set container.
        
        \return This method returns the number of events that were
        removed.
    */
    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);

    /** Convenience method used to dequeue the next set of events for
        scheduling.

        This method is used to provide necessary implementation to
        interface with the MUSE scheduler.  This method dequeues the
        next batch of the concurrent events for processing by a given
        agent.

        \param[out] events The container to which all the events to be
        processed is to be added.
    */
    void dequeueNextAgentEvents(muse::EventContainer& events);
    
    /** Convenience method for debugging/troubleshooting.

        \return The highest timestamp from the events in the multi-set bottom.
        If no events are present this method returns TIME_INFINITY.
    */
    muse::Time maxTime() const; 
    
    /** Convenience method for debugging/troubleshooting.

        \return The minimum timestamp from the events in the multi-set bottom.
        If no events are present this method returns TIME_INFINITY.
    */
    muse::Time findMinTime() const;
    
    /** Convenience method to check if the entries in the bottom are
        sorted correctly. This method is purely used for
        troubleshooting/debugging.
    */
    void validate();

    /** Obtain the count of events.

        \return The sum of events in the multi-set bottom of the Ladder queue.
    */
    inline size_t size() const { return sel.size(); }

    // The method below is purely for troubleshooting one scenario
    // where an event would get stuck in the ladder and not get
    // scheduled correctly.
    bool haveBefore(const Time recvTime) const;

    /** Method to determine the range of receive time values currently
        in bottom.  This value is used to decide if it is worth moving
        events from bottom into the ladder.

        \return The difference in maximum and minimum receive
        timestamp of events in the bottom. This value is zero if all
        events have the same receive time. If the bottom is empty,
        then this method also returns zero.
    */
    muse::Time getTimeRange() const {
        if (sel.empty()) {
            return 0;
        }
        return ((*sel.rbegin())->getReceiveTime() -
                (*sel.begin())->getReceiveTime());
    }

    /** Determine bucket width to move bottom into ladder.

        This method is invoked only when the ladder is empty and the
        bottom is long and needs to be moved into the ladder.  This
        method must compute and return the preferred bucket width.

        \note If the bottom is empty this method returns bucket width
        of 0.
     */
    double getBucketWidth() const;
    
protected:
    // Currently there are no protected members in this class
    void print(std::ostream& os = std::cout) const;
private:
    // The multi-set container that stores events.
    EventMultiSet sel;
};

/** Class that represents one rung in the ladder queue.

    The rung uses the same strategy for receive time-based
    bucket creation as the regular ladder queue.  
*/
class Rung {
public:
    /** The constructor to create an empty rung.

        The constructor merely initializes all the instance variables
        to default initial values to create an empty rung.
    */
    Rung() : rStartTS(TIME_INFINITY), rCurrTS(TIME_INFINITY),
             bucketWidth(0), currBucket(0), rungEventCount(0) {
        LQ_STATS(maxBkts = 0);
    }
    
    /** Convenience constructor to create a rung using events from the
        top rung.

        This is a delegating constructor that delegates the actual
        tasks to the overloaded constructor.

        \param[in] top The top bucket from where the events are to be
        created.
    */
    explicit Rung(Top& top);
    
    /** Convenience constructor to create a rung with events from a
        given bucket.
        
        \param[in,out] bkt The bucket from where events are to be
        moved into this newly created rung.  After this operation data
        in the bucket is cleared.

        \param[in] rStart The start time for this rung.

        \param[in] bucketWidth The delta in receive time for each
        bucket in this rung.  The bucketWidth must be > 0.
    */
    Rung(Bucket&& bkt, const Time rStart, const double bucketWidth);
    
    /** Convenience constructor to create a rung with events from a
        given EventVector.
        
        \param[in,out] list The vector from where events are to be
        moved into this newly created rung. After this operation data
        in the vector is cleared.

        \param[in] rStart The start time for this rung.

        \param[in] bucketWidth The delta in receive time for each
        vector in this rung. The bucketWidth must be > 0.
    */
    Rung(EventVector&& list, const Time rStart, const double bucketWidth);
    
    /** Convenience constructor to create a rung with events from a
        given multi-set container.
        
        \param[in,out] set The multi-set container from where events are to be
        moved into this newly created rung. After this operation data
        in the container is cleared.

        \param[in] rStart The start time for this rung.

        \param[in] bucketWidth The delta in receive time for each
        container in this rung. The bucketWidth must be > 0.
    */
    Rung(EventMultiSet&& set, const Time rStart, const double bucketWidth);

    /** Remove the next bucket in this rung for moving to another rung
        in the ladder.

        This method must be used to remove the next bucket from this
        rung. The bucket is logically removed (or moved) out of this
        rung.

        \param[out] bktTime The simulation receive time associated
        with the bucket being moved out.
    */
    Bucket&& removeNextBucket(muse::Time& bktTime);
    
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
        
        \return The bucket width for this rung.
    */
    double getBucketWidth() const { return bucketWidth; }

    /** The current bucket value in this ladder queue.

        The current minimum time of events that can be added to this
        rung of the ladder queue.

        \return The minimum timestamp of events that can be added to
        the rung of this ladder queue.
    */
    muse::Time getCurrTime() const { return rCurrTS; }

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
                     LQ_STATS(COMMA Avg& ceScanRung));

    /** Remove all events for a given receiver agent in this rung.

        This is a convenience method that removes all events for a
        given receiver agent in this rung.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver
               LQ_STATS(COMMA Avg& ceScanRung));
    
    /** Check to ensure that the number of events in various buckets
        matches the count instance variable.

        This method is used only for troubleshooting/debugging
        purposes. If counts don't match then assert fails in this
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

    /** Return the current bucket list size, which indicates the net
        number of buckets in this rung.  This information is typically
        used to report statistics at the end.

        \return The net number of buckets in this rung of the ladder.
     */
    int getBucketListSize() const {
        return bucketList.size();
    }
	
protected:

private:
    /** The lowest timestamp event that can be added to this rung.
        This value is set when a rung is created and is never changed
        during the lifetime of this rung.
    */
    muse::Time rStartTS;
    
    /** The timestamp of the lowest event that can be currently added
        to this rung. This value logically starts with rStartTS and
        grows to the time stamp of last bucket in this rung as buckets
        are dequeued from this rung.
    */
    muse::Time rCurrTS;
    
    /** The width of the bucket in simulation receive time
        differences. This value can be fractional.
    */
    double bucketWidth;
    
    /** The index of the current bucket on this rung to which events
        can be added.  This is also the next bucket that will be
        dequeued from the rung.
    */
    size_t currBucket;
    
    /** The vector containing the set of Buckets.
    */
    std::vector<Bucket> bucketList;
    
    /** Total number of events still present in this rung. This is
        used to report size and check for empty quickly.
    */
    int rungEventCount;
    
    /** Statistics object to track the maximum number of buckets used
        in this rung
    */
    LQ_STATS(size_t maxBkts);
};

/** The top-level ladder queue

    <p>This class represents the top-level ladder queue class
    that interfaces with the MUSE scheduler. This class implements
    the top-level logic associated with ladder queue to enqueue,
    dequeue, and cancel events from the ladder queue.</p>
*/
class LadderQueue : public EventQueue {
public:
    LadderQueue() : EventQueue("LadderQueue"), nRung(0), ladderEventCount(0) {
        ladder.reserve(MaxRungs);
        LQ_STATS(ceTop    = ceLadder   = ceBot  = 0);
        LQ_STATS(insTop   = insLadder  = insBot = 0);
        LQ_STATS(maxRungs = maxBotSize = 0);
    }
    
    /** The destructor.

        Currently the destructor does not have anything special to do
        as the different encapsulated objects handle all the necessary
        clean-up.
    */
    ~LadderQueue();

    /** Enqueue an event into the laadder queue.

        Depending on the scenario the event is appropriately added to
        one of: top, ladder rung, or the bottom.
        
        \param[in] e The event to be enqueued for scheduling in the
        ladder queue.
    */
    void enqueue(muse::Event* e);
    
    /** Dequeue an event from the ladder queue.

        This method removes the highest priority event from Bottom. 
        If the Bottom is empty, it is re-populated with events. 
        
    */
    muse::Event* dequeue();
    
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

        \param[in] agent The agent being added. This pointer is not
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
    
    /** This method is used to provide necessary implementation to
        interface with the MUSE scheduler. This method dequeues the
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
        scheduled. This pointer is not used.

        \param[in] event The event to be scheduled. This simply calls
        the overloaded enqueue method. The reference count on the
        event is increased by this method to account for this event
        being present in the ladder queue.
    */
    virtual void enqueue(muse::Agent* agent, muse::Event* event);
    
    /** Enqueue a batch of events

        This API to schedule a block of events. This API is typically
        used after a rollback.

        \param[in] agent The receiver agent for which the event is
        scheduled. This pointer is not used.

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
        method writes statistics only if LQ_STATS macro is enabled.
        
        \param[out] os The output stream to which the statistics are
        to be written.
    */
    virtual void reportStats(std::ostream& os);

    /** The maximum number of rungs that are normally created in the
        ladder queue.  The default value for this set to 8 based on
        the value suggested by Tang et. al. in the original Ladder
        Queue paper.  However, this value can make some difference in
        the overall performance and possibly fine tuned to suit the
        application needs based on the concurrency and number of
        events in the model.
    */
    static size_t MaxRungs;
    
protected:
    /** Check and create rungs in the ladder and return the next
        bucket of events from the ladder.

        This method implements the corresponding recurseRung method
        from the LQ paper. Refer to the paper for the details.
    */
    Bucket&& recurseRung();
    
    /** This is a convenience method that is used to move events from
        the ladder into bottom.

        This method moves events from the current bucket in the last
        rung of the ladder into the bottom. If this method is called
        when bottom is not empty, it does not perform any operation
        and returns immediately. If the ladder does not have any
        events, but the top has events, then this method first moves
        events from top-rung into the ladder and then removes events
        from the last rung into the bottom-rung.
    */
    void populateBottom();
    
    /** Method to create a new ladder rung from the current bottom.

        This method should be called only when the following 2
        conditions are met:

        1. Length of bottom is > LQ2T_THRESH
        
        2. The bottom has events that are at different time stamps --
           that is bottom.getTimeRange() > 0.

        \return This method returns the index of the rung created so
        that the caller can readily work with that rung.
    */
    int createRungFromBottom();
    
private:
    Top top;
    
    /** The ladder in the queue. The ladder consists of a set of
        rungs. The currently used rung in the ladder is indicated by
        the nRung instance variable. If the ladder is empty, then
        nRung is (or should be) 0
    */
    std::vector<Rung> ladder;

    /** The currently used last rung in the ladder queue. If the
        ladder is empty, then nRung is (or should be) 0. Otherwise
        this value is (or should be) in the range 0 < nRung <=
        ladder.size(). Rungs below nRung are not used and they do not
        contain any events to be scheduled.
    */
    size_t nRung;

    /** Instance variable to track the current number of pending
        events in all the rungs of the ladder. This is a convenience
        instance variable to quickly detect pending events in the
        ladder without having to iterate through each rung.
    */
    int ladderEventCount;
    
    Bottom bottom;
    // HeapBottom bottom;
    // MultiSetBottom bottom;

    LQ_STATS(Avg ceTop);
    LQ_STATS(Avg ceBot);
    LQ_STATS(Avg ceLadder);

    LQ_STATS(Avg ceScanTop);
    LQ_STATS(Avg ceScanLadder);

    /** The ceScanBot statistic tracks size of bottom rung scanned
        when at one (or more) events were canceled from bottom.
    */
    LQ_STATS(Avg ceScanBot);
    
    /** The ceNoCanScanBot statistic tracks size of bottom rung
        scanned but did not cancel any events.
    */
    LQ_STATS(Avg ceNoCanScanBot);
    
    LQ_STATS(int insTop);
    LQ_STATS(int insLadder);
    LQ_STATS(int insBot);
    LQ_STATS(size_t maxRungs);
    LQ_STATS(Avg avgBktCnt);
    LQ_STATS(Avg botLen);
    LQ_STATS(Avg avgBktWidth);
    
    /** Gauge to track the number of events and times bottom was
        redistributed to the last rung of the ladder.

        Redistributing bottom to the ladder ensures that the bottom
        does not get too long. But it is an expensive operation
        because all the sorting that was done is lost. So it is a
        balance and we track and report this number for reference.
    */
    LQ_STATS(Avg botToRung);
    
    /** Gauge to track the maximum length of bottom. The length of
        bottom plays an important role in the overall performance of
        the ladder queue.
    */
    LQ_STATS(size_t maxBotSize);
};

END_NAMESPACE(muse)

#endif
