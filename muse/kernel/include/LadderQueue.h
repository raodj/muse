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

#include <forward_list>
#include <queue>
#include <vector>
#include <typeinfo>
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

// The maximum number of rungs permitted in the ladder.
#define MAX_RUNGS 8

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
using EventList = std::forward_list<muse::Event*>;

class ListBucket {
public:
    ListBucket() : count(0) {}
    ListBucket(ListBucket&& src) : list(std::move(src.list)),
                           count(std::move(src.count)) {
        src.count = 0;
    }
    ~ListBucket();

    using iterator = EventList::iterator;
    using const_iterator = EventList::const_iterator;

    void push_front(muse::Event* event) {
        list.push_front(event);
        count++;
    }

    muse::Event* front() const {
        return (!list.empty() ? list.front() : NULL);
    }

    muse::Event* pop_front() {
        muse::Event* retVal = list.front();
        list.pop_front();
        count--;
        return retVal;
    }

    void insert_after(ListBucket::iterator pos, muse::Event* event) {
        list.insert_after(pos, event);
        count++;
    }

    size_t size() const { return count; }

    bool empty() const { return list.empty(); }

    ListBucket::iterator begin() { return list.begin(); }
    ListBucket::const_iterator cbegin() { return list.cbegin(); }

    ListBucket::iterator end() { return list.end(); }
    ListBucket::const_iterator cend() { return list.cend(); }
    
    int remove_after(muse::AgentID sender, const Time sendTime,
                     const bool sorted = false);

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
    EventList list;
    size_t count;
};

class VectorBucket {
public:
    VectorBucket() : count(0) {}
    VectorBucket(VectorBucket&& src) : list(std::move(src.list)),
                                       count(std::move(src.count)) {
        src.count = 0;
    }
    ~VectorBucket();

    using iterator = EventVector::iterator;
    using const_iterator = EventVector::const_iterator;
    using reverse_iterator = EventVector::reverse_iterator;

    void push_front(muse::Event* event) {
        list.push_back(event);
        count++;
    }

    muse::Event* front() const {
        return (!list.empty() ? list.back() : NULL);
    }

    muse::Event* pop_front() {
        muse::Event* retVal = list.back();
        list.pop_back();
        count--;
        return retVal;
    }

    void push_back(VectorBucket&& bucket);
    
    void insert_after(VectorBucket::iterator pos, muse::Event* event) {
        list.insert(pos + 1, event);
        count++;
    }

    void insert_after(VectorBucket::reverse_iterator pos, muse::Event* event) {
        const size_t idx = list.size() - (pos - rbegin());
        list.insert(list.begin() + idx, event);
        count++;
    }
    
    size_t size() const { return count; }

    bool empty() const { return list.empty(); }

    VectorBucket::iterator begin() { return list.begin(); }
    VectorBucket::const_iterator cbegin() const { return list.cbegin(); }
    VectorBucket::reverse_iterator rbegin() { return list.rbegin(); }

    VectorBucket::iterator end() { return list.end(); }
    VectorBucket::const_iterator cend() const { return list.cend(); }
    VectorBucket::reverse_iterator rend() { return list.rend(); }

    
    int remove_after(muse::AgentID sender, const Time sendTime,
                     const bool sorted = false);

    /** Remove all events in thisd vector bucket for a given receiver
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
    EventVector list;
    size_t count;
};

// using Bucket = ListBucket;
using Bucket = VectorBucket;

class Top {
    friend class Rung;
    friend class LadderQueue;
public:
    // Constructor to initialize the instance variables.
    Top() { reset(); }
    ~Top();
    void add(muse::Event* event);
    bool empty() const { return events.empty(); }
    Time getStartTime() const { return topStart; }
    Time getMinTime() const { return minTS; }
    Time getMaxTime() const { return maxTS; }
    
    bool contains(const Time ts) const {
        return (ts >= minTS) && (ts <= maxTS);
    }
    double getBucketWidth() const {
        DEBUG(std::cout << "minTS=" << minTS << ", maxTS=" << maxTS
                        << ", size=" << size() << std::endl);
        return (maxTS - minTS + size() - 1.0) / size();
    }

    int size() const { return events.size(); }

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
    void reset(const Time topStart = 0);

private:
    muse::Time minTS;
    muse::Time maxTS;
    muse::Time topStart;
    Bucket events;
};

class Bottom {
    friend class LadderQueue;   // NOTE: uses sel directly
public:
    void enqueue(Bucket&& bucket) {
        // Delegate to overloaded method to handle different scenarios
        // depending on whether linked-list or vector is used for Buckets
        enqueue(std::move(bucket), sel);
    }
    void enqueue(muse::Event* event) {
        // Delegate to overloaded method to handle different scenarios
        // depending on whether linked-list or vector is used for Buckets
        enqueue(event, sel);        
    }

    muse::Event* pop_front() { return sel.pop_front(); }
    muse::Event* front() const { return sel.front(); }

    bool empty() const {
        return sel.empty();
    }

    int remove_after(muse::AgentID sender, const Time sendTime);
    
    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);
    
    void dequeueNextAgentEvents(muse::EventContainer& events);
    muse::Time maxTime() const;  // purely for debugging
    muse::Time findMinTime() const; // purely for debugging
    
    void validate();

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
        queue.  This comparsion reverses the sort order -- it places
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
 
    bool haveBefore(const Time recvTime) const {
        return sel.haveBefore(recvTime);
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
    Bucket sel;
};

class HeapBottom {
    friend class LadderQueue;   // NOTE: uses sel directly
public:
    void enqueue(Bucket&& bucket);
    void enqueue(muse::Event* event);

    muse::Event* pop_front();
    muse::Event* front() const { return sel.front(); }

    bool empty() const { return sel.empty();  }

    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);

    void dequeueNextAgentEvents(muse::EventContainer& events);
    muse::Time maxTime() const;  // purely for debugging
    muse::Time findMinTime() const; // purely for debugging
    
    void validate();

    inline size_t size() const { return sel.size(); }

    bool haveBefore(const Time recvTime) const;
    
protected:
    // Currently there are no protected members in this class
    void print(std::ostream& os = std::cout) const;
private:
    EventVector sel;
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
    void enqueue(Bucket&& bucket);
    void enqueue(muse::Event* event);

    muse::Event* pop_front();
    muse::Event* front() const { return *sel.cbegin(); }

    bool empty() const { return sel.empty();  }

    int remove_after(muse::AgentID sender, const Time sendTime);

    /** Remove all events for a given receiver agent in the bucket
        encapsulated by this object.

        This is a convenience method that removes all events for a
        given receiver agent in this object.  This method is used to
        remove events scheduled for an agent, when an agent is removed
        from the scheduler.
    */
    int remove(muse::AgentID receiver);

    void dequeueNextAgentEvents(muse::EventContainer& events);
    muse::Time maxTime() const;  // purely for debugging
    muse::Time findMinTime() const; // purely for debugging
    
    void validate();

    inline size_t size() const { return sel.size(); }

    bool haveBefore(const Time recvTime) const;
    
protected:
    // Currently there are no protected members in this class
    void print(std::ostream& os = std::cout) const;
private:
    EventMultiSet sel;
};

class Rung {
public:
    Rung() : rStartTS(TIME_INFINITY), rCurrTS(TIME_INFINITY),
             bucketWidth(0), currBucket(0), rungEventCount(0) {}
    explicit Rung(Top& top);
    Rung(Bucket&& bkt, const Time rStart, const double bucketWidth);
    Rung(EventVector&& list, const Time rStart, const double bucketWidth);
    Rung(EventMultiSet&& set, const Time rStart, const double bucketWidth);
    
    Bucket&& removeNextBucket(muse::Time& bktTime);
    bool empty() const { return (rungEventCount == 0); }

    void enqueue(muse::Event* event);

    muse::Time getStartTime() const { return rStartTS; }

    double getBucketWidth() const { return bucketWidth; }

    muse::Time getCurrTime() const {
        return rCurrTS;
    }

    muse::Time getMaxRungTime() const {
        return rStartTS + (bucketList.size() * bucketWidth);
    }
    
    bool canContain(muse::Event* event) const;

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
    
    void validateEventCounts() const;
    void prettyPrint(std::ostream& os) const;
    
    void updateStats(Avg& avgBktCnt) const;

    bool isCurrBucketEmpty() const {
        return (currBucket >= bucketList.size() ||
                bucketList[currBucket].empty());
    }
    
    bool haveBefore(const Time recvTime) const;
    
protected:

private:
    muse::Time rStartTS;
    muse::Time rCurrTS;
    double bucketWidth;
    // The current bucket on this rung
    size_t currBucket;
    std::vector<Bucket> bucketList;
    // Total number of events still present in this rung.
    int rungEventCount;
    // The maximum number of buckets used in this rung
    LQ_STATS(size_t maxBkts);
};

class LadderQueue : public EventQueue {
public:
    LadderQueue() : EventQueue("LadderQueue"), ladderEventCount(0) {
        ladder.reserve(MAX_RUNGS);
        LQ_STATS(ceTop    = ceLadder  = ceBot  = 0);
        LQ_STATS(insTop   = insLadder = insBot = 0);
        LQ_STATS(maxRungs = 0);
    }
    ~LadderQueue();

    void enqueue(muse::Event* e);
    muse::Event* dequeue();
    int remove_after(muse::AgentID sender, const Time sendTime);
    virtual bool empty() {
        return top.empty() && (ladderEventCount == 0) &&  bottom.empty();
    }

    virtual void* addAgent(muse::Agent* agent);
    virtual void removeAgent(muse::Agent* agent);
    virtual muse::Event* front();
    virtual void dequeueNextAgentEvents(muse::EventContainer& events);
    virtual void enqueue(muse::Agent* agent, muse::Event* event);
    virtual void enqueue(muse::Agent* agent, muse::EventContainer& events);
    virtual int eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                           const muse::Time sentTime);
    virtual void prettyPrint(std::ostream& os) const;

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
    
protected:
    Bucket&& recurseRung();
    void populateBottom();
    int createRungFromBottom();
    
private:
    Top top;
    std::vector<Rung> ladder;
    int ladderEventCount;
    Bottom bottom;
    // HeapBottom bottom;
    // MultiSetBottom bottom;

    LQ_STATS(Avg ceTop);
    LQ_STATS(Avg ceBot);
    LQ_STATS(Avg ceLadder);

    LQ_STATS(Avg ceScanTop);
    LQ_STATS(Avg ceScanBot);
    LQ_STATS(Avg ceScanLadder);
    
    LQ_STATS(int insTop);
    LQ_STATS(int insLadder);
    LQ_STATS(int insBot);
    LQ_STATS(size_t maxRungs);
    LQ_STATS(Avg avgBktCnt);
    LQ_STATS(Avg botLen);
    LQ_STATS(Avg avgBktWidth);
};

END_NAMESPACE(muse)

#endif
