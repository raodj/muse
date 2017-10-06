#ifndef THREE_TIER_HEAP_EVENT_QUEUE_H
#define THREE_TIER_HEAP_EVENT_QUEUE_H

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

#include <vector>
#include <stack>
#include <algorithm>
#include "Avg.h"
#include "EventQueue.h"

BEGIN_NAMESPACE(muse)

/** Class to encapsulate information for Tier2 entries/buckets.

	This is a simple class that encapsulates a list of events (in
	eventList) all with exactly the same receive time to a given
	agent.  These objects are cached/reused by the scheduler queue to
	reduce memory allocation operations, particularly for the
	eventList because memory management turns out to be he most
	expensive operation.
*/
class HOETier2Entry {
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
    HOETier2Entry(muse::Event* event) : recvTime(event->getReceiveTime()),
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
    }

    /** Appends events to the EventContainer list.
     *
     *  The method is used to append concurrent events to their respective
     *  position in the tier2 container.
     */
    void updateContainer(muse::Event* event){
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
    inline bool operator==(const HOETier2Entry &rhs) {
        return (this->recvTime == rhs.recvTime);
    }

    inline bool operator<(const HOETier2Entry &rhs) {
        return (this->recvTime < rhs.recvTime);
    }
    
    inline Time getReceiveTime() const {
        return recvTime;
    }
    
    inline const std::vector<muse::Event*>& getEventList() const {
        return eventList;
    }

    inline std::vector<muse::Event*>& getEventList() {
        return eventList;
    }
};

// A convenience shortcut used just in this source file
using Tier2List = std::deque<HOETier2Entry*>;

/** A three-tier-heap aka "3tHeap" or "heap-of-heap" event queue for
    managing events.

    <p>This class provides a heap-of-heap based event queue for
    managing events for simulation.  The two-tiers are organized as
    follows:</p>

    <p><u>First tier:</u> This class uses standard C++ algorithms
    (such as: \c std::make_heap, \c std::push_heap, \c std::pop_heap)
    to manage a heap of events for each agent.  The events are stored
    in a backing std::vector in each agent.  It is the same per-agent
    infrastructure as used by Fibonacci heap (implemented in AgentPQ
    class).</p>
    
    <p><u>Second tier:</u> This class specifically handles the
    necessary behavior of the second tier of operations -- that is
    scheduling of agents by maintaining heap of agents.</p>

    \note On the long run it would be better to avoid reliance on
    std::push_heap or std::pop_heap methods due to implicit dependence
    in the fixHeap() method.
*/
class ThreeTierHeapEventQueue : public EventQueue {
public:
    /** The constructor for the TwoTierHeapEventQueue.

        The default (and only) constructor for this class.  The
        constructor does not have any specific task to perform other
        than set a suitable identifier in the base class.
    */    
    ThreeTierHeapEventQueue();

    /** The destructor.

        The destructor does not have any special tasks to perform.
        All the dynamic memory management is handled by the standard
        containers (namely std::vector) that is used internally by
        this class.
    */
    ~ThreeTierHeapEventQueue();

    /** Add/register an agent with the event queue.

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

        <p>This method implements the corresponding API method in the
        class.  Refer to the API documentation in the base class for
        intended functionality.</p>

        <p>This method removes all events scheduled for the specified
        agent in its internal data structures.</p>
        
        \param[in,out] agent A pointer to the agent whose events are
        to be removed from the vector managed by this class.
    */
    void removeAgent(muse::Agent* agent) override;
    
    /** Determine if the event queue is empty.
        
        This method implements the base class API to report if any
        events are pending to be processed in the event queue.

        \return This method returns true if the event queue of the
        top-agent is logically empty.
    */    
    virtual bool empty() {
        return (agentList.empty() || top()->tier2->empty());
    }

    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in this event queue, without de-queuing the
        event.

        \note The event returned by this method is not dequeued.
        
        \return A pointer to the next event to be processed.  If the
        queue is empty then this method returns NULL.
    */
    virtual muse::Event* front();

    /** Method to obtain the next batch of events to be processed by
        one agent.

        <p>In MUSE agents are scheduled to process all events at a
        given simulation time.  The next concurrent events (i.e.,
        events with the same receive time) with the lowest time stamp
        are to be placed in the supplied event container.  The event
        container is then passed to the corresponding agent for
        further processing.</p>

        <p>This method essentially delegates the dequeue process to
        the agent with the next lowest timestamp.  Once the agent has
        been dequeued, this method fixes the heap by placing the
        top-agent in its appropriate location in the heap.</p>

        \param[out] events The event container in which the next set
        of concurrent events are to be placed.  Note that the order of
        concurrent events in the event container is unspecified.
    */
    virtual void dequeueNextAgentEvents(muse::EventContainer& events);

    /** Enqueue a new event.

        This method must be used to enqueue/add an event to this event
        queue.  Once added the reference count on the event is
        increased.  This method adds the event to the specified agent.
        Next this method fixes the heap to ensure that the agent with
        the least-time-stamp is at the top of the heap.  This method
        essentially uses an internal helper method to accomplish its
        tasks.

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
        rollback.  Next this method fixes the heap to ensure that the
        agent with the least-time-stamp is at the top of the heap.
        This method uses an internal helper method to accomplish its
        tasks.

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

    /** Dequeue all events sent by an agent after a given time.

        This method implements the base class API method.  This method
        can be used to remove/erase events sent by a given agent after
        a given simulation time.  This API is needed to cancel events
        during a rollback.  Next this method fixes the heap to ensure
        that the agent with the least-time-stamp is at the top of the
        heap.

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

        This is a convenience method that is used primarily for
        troubleshooting purposes.  This method prints all the events
        in this queue, with each event on its own line.

        \param[out] os The output stream to which the contents of the
        queue are to be written.
    */
    virtual void prettyPrint(std::ostream& os) const;

    /** Method to report aggregate statistics.

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
    
    /** Convenience method to remove events.

        This is an internal convenience method that is used to remove
        the front (i.e., events with lowest timestamp) event list from this
        queue.
    */
    void pop_front(muse::Agent* agent);
    
    /** Convenience method to obtain the top-most or front agent.

        This method can be used to obtain a pointer to the top/front
        agent -- that is, the agent with the lowest timestamp event to
        be scheduled next.

        \return A pointer to the top-most agent in this heap.
    */
    inline muse::Agent* top() {
        return agentList.front();
    }
        
    /** Convenience method to get the top-event time for a given
        agent.

        This method returns the top event time in the vector of events queue.
        If agent's vector of event queue is empty, then it returns infinity. 
        
        \return The receive time of top event's recv time or
        TIME_INFINITY if vector is empty.
    */
    inline muse::Time getTopTime(const muse::Agent* const agent) const {
        return agent->tier2->empty() ? TIME_INFINITY :
            agent->tier2->front()->getReceiveTime();
    }
    
    /** Comparator method to sort events in the heap.

        This is the comparator method that is passed to various
        standard C++ algorithms to organize events as a heap.  This
        comparator method gives first preference to receive time of
        events.  Tie between two events with the same recieve time is
        broken based on the receiver agent ID.

        \param[in] lhs The left-hand-side event to be used for
        comparison.  This parameter cannot be NULL.

        \param[in] rhs The right-hand-side event to be used for
        comparison. This parameter cannot be NULL.

        \return This method returns if lhs < rhs, i.e., the lhs event
        should be scheduled before the rhs event.
    */
    inline bool compare(const Agent *lhs, const Agent * rhs) const {
        return getTopTime(lhs) >= getTopTime(rhs);
    }
    
    /** Comparator method to sort events in the heap.

        This is the comparator method that is passed to various
        standard C++ algorithms to organize events as a heap.  This
        comparator method gives first preference to receive time of
        events.  Tie between two events with the same recieve time is
        broken based on the receiver agent ID.

        \param[in] lhs The left-hand-side event to be used for
        comparison.  This parameter cannot be NULL.

        \param[in] rhs The right-hand-side event to be used for
        comparison. This parameter cannot be NULL.

        \return This method returns if lhs < rhs, i.e., the lhs event
        should be scheduled before the rhs event.
    */
    inline static bool lessThan(const HOETier2Entry& lhs,
                                const muse::Event* const event) {
        return (lhs.getReceiveTime() < event->getReceiveTime());
    }

    /** Comparator method to sort events in the heap.

        This is the comparator method that is passed to various
        standard C++ algorithms to organize events as a heap.  This
        comparator method gives first preference to receive time of
        events.  Tie between two events with the same recieve time is
        broken based on the receiver agent ID.

        \param[in] lhs The left-hand-side event to be used for
        comparison.  This parameter cannot be NULL.

        \param[in] rhs The right-hand-side event to be used for
        comparison. This parameter cannot be NULL.

        \return This method returns if lhs < rhs, i.e., the lhs event
        should be scheduled before the rhs event.
    */
    inline static bool lessThanPtr(const HOETier2Entry* const lhs,
                                   const muse::Event* const event) {
        return (lhs->getReceiveTime() < event->getReceiveTime());
    }
    
    /** The getNextEvents method.

        This method is a helper that will grab the next set of events
        to be processed for a given agent.  This method is invoked in
        dequeueNextAgentEvents() method in this class.
		
        \param[out] container The reference of the container into
        which events should be added.        
    */
    void getNextEvents(Agent* agent, EventContainer& container);
    
        /** Obtain the current index of the agent from it's
        cross-reference.

        This method is a refactored utility method that has been
        introduced to streamline the code.  This method essentially
        obtains the index position of the given agent in the agentList
        vector from the agent's fibHeapPtr corss-reference.  This
        cross-reference is consistently updated by the various methods
        in this class to enable rapid access to the location of the
        agent.

        \param[in] agent The agent whose index value in the agentList
        is to be determined.

        \return The index position of the agent in the agentList
        vector (if all checks pass).
    */
    size_t getIndex(muse::Agent *agent) const;
    
    /** Update position of agent in the scheduler's heap.

        This is an internal helper method that is used to update the
        position of an agent in the scheduler's heap.  This method
        essentially performs sanity checks, uses the fixHeap() method
        to update position of the agent, and updates cross references
        for future use.

        \param[in] agent The agent whose position in the heap is to be
        updated.  This pointer cannot be NULL.

        \return This method returns the updated index position of the
        agent in agentList (the vector that serves as storage for the
        heap).
    */
    size_t updateHeap(muse::Agent* agent);
    
    /** Fix-up the location of the agent in the heap.

        This method can be used to update the location of an agent in
        the heap.

        \note The implementation for this method has been heavily
        borrowed from libstdc++'s code base to ensure that heap
        updates are consistent with std::make_heap API.
        Unfortunately, this does imply that there is a chance this
        method may be incompatible with future versions.

        \param[in] currPos The current position of the agent in the
        heap whose position is to be updated.  This value is the index
        position of the agent in the agentList vector.
        
        \return This method returns the new position of the agent in
        the agentList vector.
    */
    size_t fixHeap(size_t currPos);

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
    HOETier2Entry* makeTier2Entry(muse::Event* event) {
        if (!tier2Recycler.empty()) {
            HOETier2Entry* entry = tier2Recycler.back();
            tier2Recycler.pop_back();
            entry->reset(event);
            return entry;
        }
        return new HOETier2Entry(event);
    }
    
private:
    /** The backing storage for events managed by this class.

        This vector contains the list of agents being managed by the
        class.  The agents in the vector are stored and maintained as
        a heap.  The heap is created and managed using standard C++
        algorithms, namely: \c std::make_heap, \c std::push_heap, and
        \c std::pop_heap.
    */
    std::vector<muse::Agent*> agentList;

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
    std::deque<HOETier2Entry*> tier2Recycler;

    
    /** Default, empty Tier2List to handle removal of agents.

        This is a default/empty tier-2 list that is used to streamline
        removal of agents.  As agents are removed in the removeAgent
        method, their 2nd tier queue is substituted with this
        default/empty list.  This enables updating position of agents
        within this heap without causing memory issues.
    */
    Tier2List EmptyT2List;
};

END_NAMESPACE(muse)

#endif
