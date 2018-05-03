#ifndef THREE_TIER_SKIP_MT_QUEUE_H
#define THREE_TIER_SKIP_MT_QUEUE_H

//---------------------------------------------------------------------------
// 
// This uses a lock free priority queue by Linden and Jonsson (2013)
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


#include <vector>
#include <stack>
#include <algorithm>
#include "Avg.h"
#include "EventQueueMT.h"
#include "mpi-mt-shm/LockFreePQ.h"

BEGIN_NAMESPACE(muse);

/** Class to encapsulate information for Tier2 entries/buckets.

	This is a simple class that encapsulates a list of events (in
	eventList) all with exactly the same receive time to a given
	agent.  These objects are cached/reused by the scheduler queue to
	reduce memory allocation operations, particularly for the
	eventList because memory management turns out to be the most
	expensive operation.
*/
class HOETier2EntryMT {
private:
    /** The receive time of events in this tier2 entry. Note that all
        the events in this entry are must/are concurrent -- that is,
        destined for the same agent at the same time.
     */
    Time recvTime;

    /** The list of entities in this HOE entry class */
    std::vector<muse::Event*> eventList;

public:
    /** Constructor to create a tier2 entry with 1 initial event in
        it.

        \param[in] event The event to be added to this tier2 entry.
        The receive time of the event is used as the receive time
        value.
    */
    HOETier2EntryMT(muse::Event* event) : recvTime(event->getReceiveTime()),
                                        eventList(1, event) {}

    /** Reset the information in this tier2 entry.

        This method is synonymous to the constructor except, it is
        used to reset/recycle an existing tier2 entry.

        \param[in] event The event to be added to this tier2 entry.
        The receive time of the event is used as the receive time
        value.
    */
    void reset(muse::Event* event) {
        recvTime = event->getReceiveTime();
        eventList.clear();
        eventList.emplace_back(event);
        removed = false;
    }

    /** Appends events to the EventContainer list.
     *
     *  The method is used to append concurrent events to their respective
     *  position in the tier2 container.
     */
    void updateContainer(muse::Event* event){
        ASSERT(!entryGuard.try_lock()); // can't insert if caller didn't lock
        eventList.emplace_back(event);
    }

    /** Obtain pointer to the first event in this list.

        \return Pointer to the first event in this list. This return
        value cannot/should-not be NULL.
    */
    inline muse::Event* getEvent() const {
        return eventList.front();
    }
  
    /** \brief compares the receive times of events
        
        The method is used to determine whether or not an event
        already exists in the tier2 container.
        
        \returns True if lhs receiveTime is equal to rhs receiveTime
    */
    inline bool operator==(const HOETier2EntryMT &rhs) {
        return (this->recvTime == rhs.recvTime);
    }

    inline bool operator<(const HOETier2EntryMT &rhs) {
        return (this->recvTime < rhs.recvTime);
    }
    
    inline Time getReceiveTime() const {
        return recvTime;
    }
    
    inline const std::vector<muse::Event*> getEventList() const {
        return eventList;
    }

    inline std::vector<muse::Event*> getEventList() {
        return eventList;
    }
    
    /** Ensures thread safe insert and removal of this 3rd tier queue */
    std::mutex entryGuard;
    
    /** This flag is set when this entry is used up and marked for recycling.
        This is used to prevent deadlock, if we lock entryGuard and this is
        true, we know we missed this entry and need to insert into a new one.
        
        Must be used alongside the entryGuard to ensure thread safe operation
     */
    bool removed = false;
};

/** A three-tier-skip-mt aka "3tSkipMT" event queue for managing events.

    <p>This queue is a thread safe, concurrent data structure, meaning multiple
    threads can operate on it directly with no loss in expected behavior.</p>
    
    <p>This class provides a priority queue-of-list-of-lists based event queue
    for managing events for simulation.  The three-tiers are organized
    as follows:</p>

    <p><u>First tier:</u> This uses a lock free, thread safe priority queue
    based on the skip list data structure by Linden and Jonsson (2013).  
    Each entry represents an agent, with the "top" agent being the agent
    with the lowest timestamp of events. </p>
    
    <p><u>Second tier:</u> This tier specifically handles the
    necessary behavior of the second tier of operations -- that is
    scheduling of events by maintaining a sorted list-of-list of
    events. This tier is also thread safe and uses the same lock free
    queue as tier one.</p>

    <p><u>Third tier:</u> This tier contains a list of concurrent
    events -- that is, events scheduled for a given agent at a given
    time.  The third tier is implemented by the HOETier2Entry
    objects which simply holds a normal vector of events since it 
    does not need to be sorted.</p>
*/
class ThreeTierSkipMTQueue : public EventQueueMT {
public:
    /**
     *  The key type for agents in a ThreeTierSkipMTQueue
     */
    // todo(deperomm): This typedef is copied in Agent.h, not linked to
    //                 same with Tier2ListMT below
    typedef std::pair<muse::Time, muse::AgentID> AgentKey;
    
    /**
     * Custom comparator on type AgentKey for sorting in LockFreePQ
     * 
     * @param k1 - the left AgentKey to compare
     * @param k2 - the right AgentKey to compare
     * @return if left < right (left == right returns false)
     */
    struct AgentComp {
        inline bool operator() (const AgentKey& k1, const AgentKey& k2)
        {
            if (k1.first < k2.first) {
                return true;
            }
            if (k1.first == k2.first) {
                return k1.second < k2.second;
            }
            return false;
        }
    };
    
    /**
     * The type for the top level container of agents
     */
    typedef LockFreePQ<AgentKey, muse::Agent*, AgentComp> AgentList;
    
    /**
     * The type for the list of tier2 entries
     */
    typedef LockFreePQ<muse::Time, HOETier2EntryMT*> Tier2ListMT;
    
    /** The constructor for the ThreeTierSkipMTQueue.

        The default (and only) constructor for this class.  The
        constructor does not have any specific task to perform other
        than set a suitable identifier in the base class.
    */    
    ThreeTierSkipMTQueue();

    /** The destructor.

        The destructor does not have any special tasks to perform.
        All the dynamic memory management is handled by the standard
        containers (namely std::vector) that is used internally by
        this class.
    */
    ~ThreeTierSkipMTQueue();

    /** Add/register an agent with the event queue.
        
        ===== This Methis is NOT Thread Safe =====
        Agents should be shared between all threads, and agents must be
        inserted during a sequential portion of the simulation
        
        <p>This method implements the corresponding API method in the
        class.  Refer to the API documentation in the base class for
        intended functionality.</p>

        <p>This class uses the supplied agent pointer to setup the
        list of agents managed and scheduled by this class.</p>

        \param[in,out] agent A pointer to the agent to be registered.
        This value is not used.

        \return This method returns the iterator to the position of
        the agent in its internal vector as a cross-reference to be
        stored in an agent.
    */    
    virtual void* addAgent(muse::Agent* agent);

    /** Remove/unregister an agent with the event queue.
        
        ===== This Methis is NOT Thread Safe =====
        Agents should be shared between all threads, and agents must be
        removed during a sequential portion of the simulation

        <p>This method implements the corresponding API method in the
        class.  Refer to the API documentation in the base class for
        intended functionality.</p>

        <p>This method removes all events scheduled for the specified
        agent in its internal data structures.</p>
        
        \param[in,out] agent A pointer to the agent whose events are
        to be removed from the vector managed by this class.
    */
    void removeAgent(muse::Agent* agent) override;
    
    /**
     * This method pops the next agent off the top level queue, thus providing
     * exclusive dequeue rights to this thread for that agent.
     * 
     * This agent must be returned back into the queue when processing of it is
     * done via EventQueue::returnAgent(). 
     * 
     * @return the next agent to be processed by the sim
     */
    muse::Agent* popNextAgent();

    /** Method to obtain the next batch of events to be processed by one agent
        
        This agent must have been popped off the queue already such that
        the thread calling this has exclusive access to call this method on
        the given agent. This is done by first calling popNextAgent().

        <p>In MUSE agents are scheduled to process all events at a
        given simulation time.  The next concurrent events (i.e.,
        events with the same receive time) with the lowest time stamp
        are to be placed in the supplied event container.  The event
        container is then passed to the corresponding agent for
        further processing.</p>

        <p>This method essentially delegates the dequeue process to
        the agent with the next lowest timestamp. The agent is
        popped off the top level queue in a thread safe way so that it's
        events may be dequeued and returned while also allowing other threads
        to grab other agents and remove their events at the same time.</p>
        

        \param[out] events The event container in which the next set
        of concurrent events are to be placed.  Note that the order of
        concurrent events in the event container is unspecified.
    */
    void dequeueNextEvents(muse::Agent *agent, muse::EventContainer& events);
    
    /**
     * Puts the agent back into the top level queue for future processing
     * 
     * @param agent to insert back after done processing its events
     */
    void pushAgent(muse::Agent *agent);


    /** Enqueue a new event.

        This method must be used to enqueue/add an event to this event
        queue.  Once added, the reference count on the event is
        increased.  This method adds the event directly into the agents
        second and third tier, and then updates the top tier if the
        event lowers the agents lowest event timestamp.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.
        
        \param[in] event The event to be enqueued.  This parameter can
        never be NULL.
    */
    virtual void enqueue(muse::Agent* agent, muse::Event* event);

    /** Enqueue a batch of events.

        This method can be used to enqueue/add a batch of events to
        this event queue.  Once added the reference count on each one
        of the events is increased.  This method provides a convenient
        approach to enqueue a batch of events, particularly after a
        rollback.  Next this method updates the top tier if the
        events lower the agents lowest event timestamp.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.  Currently, this value is
        not used.

        \param[in] event The list of events to be enqueued.  This
        container can and will be be empty in certain situations.  The
        reference counts of the events in the container remains
        unmodified.  The list of events become part of the event
        queue.
    */
    virtual void enqueue(muse::Agent* agent, muse::EventContainer& events);

    /** Dequeue all events sent by an agent on or after a given time.

        This method implements the base class API method.  This method
        can be used to remove/erase events sent by a given agent on or
        after a given simulation time.  This API is needed to cancel events
        during a rollback. This method also updates the priority of the
        agent on the top level queue if necessary.

        \param[in] dest The agent whose currently scheduled events
        are to be checked and cleaned-up.  This agent must be a valid
        agent that has been registered/added to this event queue.  The
        pointer cannot be NULL.  This parameter is not used.
        
        \param[in] sender The ID of the agent whose events have to be
        removed.  This agent must be a valid agent that has been
        registered/added to this event queue.  The pointer cannot be
        NULL.

        \param[in] sentTime The time from which the events are to be
        removed.  All events (including those sent at this time) sent
        by the sender agent are removed from this event queue.

        \return This method returns the number of events actually
        removed.
    */
    virtual int eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                           const muse::Time sentTime);
    
    /** Print full contents of scheduler queue to given output stream.

        This method is NOT thread safe, and should not be called while the
        queue has active threads operating on it.
        
        This is a convenience method that is used primarily for
        troubleshooting purposes.  This method prints all the events
        in this queue, with each event on its own line.

        \param[out] os The output stream to which the contents of the
        queue are to be written.
    */
    virtual void prettyPrint(std::ostream& os) const;

    /** Method to report aggregate statistics.
        
        This method is NOT thrad safe, and has undefined behavior if called
        while other threads are actively operating on this queue.
        
        This method is invoked at the end of simulation after all
        agents on this rank have been finalized.  This method can
        report any aggregate statistics from the event
        queue. Currently, this method does not have any additional
        statistics to report.

        \param[out] os The output stream to which the statistics are
        to be written.
    */
    virtual void reportStats(std::ostream& os);
    
    
protected:
    /** Enqueue a new event.

        This method must be used to enqueue/add an event to this event
        queue.  Once added the reference count on the event is
        increased.  This method adds the event to the specified agent.
        Next this method fixes the heap to ensure that the agent with
        the least-time-stamp is at the top of the heap.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.
        
        \param[in] event The event to be enqueued.  This parameter can
        never be NULL.
    */
    virtual void enqueueEvent(muse::Agent* agent, muse::Event* event);
    
    /**
     * Used after event insertion to restructure top agent queue if necessary
     * 
     * This is expensive as it must both remove and re-insert the agent into
     * the top level queue (potential optimizations exist here), so for batch
     * event inserts only restructure with the min event after all inserts are
     * done.
     * 
     * If an event is inserted for an agent that is earlier than all other
     * events for that agent, that agent's timestamp changes its priority which 
     * may justify a restructure. If so, this method does that restructure
     * 
     * @param agent
     * @param newTime
     */
    void restructureTopQueue(muse::Agent* agent, muse::Time newTime);

    /** Convenience method to determine if an event is a future event.

        This method is a helper method used in the eraseAfter() method
        to determine if a given event is a future event from a given
        sender agent.

        \param[in] sender The sender agent to be used in comparison.

        \param[in] sendTime The reference time for comparison

        \param[in] evt The event to be checked if it is future event.

        \return This method returns true if the event is sent from a
        given sender agent and its send time is greater-or-equal to
        the given sentTime.
     */
    inline bool
    isFutureEvent(const muse::AgentID sender, const muse::Time sentTime, 
                  const muse::Event* evt) const {
        return ((evt->getSenderAgentID() == sender) &&
                (evt->getSentTime() >= sentTime));
    }

    /** Helper method to reuse tier2 entries or create a new one.

        This method is a convenience method to recycle tier2 entry
        object is available.  If the recycle bin is empty, then this
        method creates a new object.
        
        \param[in] event The event to be used to initialize and to be
        contained in the newly created tier2 entry.
        
        \return A tier2 entry initialized and containing the given
        event.
     */
    HOETier2EntryMT* makeTier2Entry(muse::Event* event) {
        tier2RecyclerLock.lock();
        if (!tier2Recycler.empty()) {
            HOETier2EntryMT* entry = tier2Recycler.back();
            tier2Recycler.pop_back();
            entry->reset(event);
            tier2RecyclerLock.unlock();
            return entry;
        }
        tier2RecyclerLock.unlock(); // recycler was empty, make a new one
        return new HOETier2EntryMT(event);
    }
    
    /**
     * Adds a tier3Entry into the recycler in a thread safe way
     * 
     * Note that the entry is not reset on insert, and must be reset when
     * removed in order to be used again
     * 
     * @param e - the tier2entry to recycle
     */
    inline void recycleTier2Entry(HOETier2EntryMT* e) {
        tier2RecyclerLock.lock();
        tier2Recycler.emplace_back(e);
        tier2RecyclerLock.unlock();
    }
    
    /**
     * Attempts to insert an event into a tier2Entry.
     * 
     * This is tricky because another thread could dequeue this entry at the
     * same time, after which we need to make a new entry and try again.
     * 
     * If we fail, it means the entry was concurrently dequeued, return false,
     * and the caller needs to make a new entry, resulting in a rollback
     * 
     * @param t2 - The tier2Entry to insert the event into
     * @param event - The event we're inserting
     * @return bool - whether or not the event was successfully inserted
     */
    bool tryTier2Insert(HOETier2EntryMT* t2e, muse::Event* event) {
        t2e->entryGuard.lock();
        if (t2e->removed) {
            t2e->entryGuard.unlock(); // avoid deadlock, dequeue thread recycles
            return false; // already been dequeued
        } else {
            t2e->updateContainer(event);
            t2e->entryGuard.unlock();
            return true;
        }
    }
    
private:
    /** The backing storage for events managed by this class.

        This vector contains the list of agents being managed by the
        class.  The agents in the vector are stored and maintained as
        a heap.  The heap is created and managed using standard C++
        algorithms, namely: \c std::make_heap, \c std::push_heap, and
        \c std::pop_heap.
    */
    AgentList *agentList;

    /** Stats object to track the average tier-2 bucket size.  This
        value is the one that primarily determines if tier-2
        operations are going to be optimal or not.  Higher this value,
        the better for this event queue.
    */
    Avg avgSchedBktSize;

    /** The number of times the fixHeap method performed heap-fixing
        operations.  This variable is fine-grained in that it
        accumulates the average number of compares that occur to fix
        up the heap of agents.  The fixHeap method has a q1 = O(log
        n1) compares.  So if this method is called m times, the
        statistics reports (q1 + q2 + ... + qm) / m.
    */
    Avg fixHeapSwapCount;

    /** The average queue size for each agent.  This value determines
        the time takes to find a bucket into which an event is to be
        inserted.
    */
    Avg agentBktCount;

    /** A stack to recycle Tier2 entries to minimize memory allocation
        calls for these small blocks used in this queue.
    */
    std::deque<HOETier2EntryMT*> tier2Recycler;
    
    /** Simple lock to allow threadsafe access to recycler stack
     */
    std::mutex tier2RecyclerLock;

    
//    /** Default, empty Tier2List to handle removal of agents.
//
//        This is a default/empty tier-2 list that is used to streamline
//        removal of agents.  As agents are removed in the removeAgent
//        method, their 2nd tier queue is substituted with this
//        default/empty list.  This enables updating position of agents
//        within this heap without causing memory issues.
//    */
//    Tier2List EmptyT2List;
    
};

END_NAMESPACE(muse);

#endif
