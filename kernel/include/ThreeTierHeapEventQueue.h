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
#include "EventQueue.h"
#include "BinaryHeapWrapper.h"
#include "BinaryHeap.h"

BEGIN_NAMESPACE(muse)

/** The Tier2Entry class creates an object that contains concurrent events.
 * The events are stored in the EventContainer and the objects are stored
 * in the tier2 container. 
 */
class Tier2Entry {
private:
    Time recvTime;
    AgentID agentID;
    muse::EventContainer eventList; 
public:
    
    Tier2Entry();
    
    Tier2Entry(muse::Event* event) {
        eventList.push_back(event);
        recvTime = event->getReceiveTime();
        agentID = event->getReceiverAgentID();
    }
    
    /** Appends events to the EventContainer list.
     *  The method is used to append concurrent events to their respective
     *  position in the tier2 container. 
     */
    void updateContainer(muse::Event* event){
        eventList.push_back(event);
    }
    
    /** \brief compares the receive times of events
     *  The method is used to determine whether or not an event already exists
     *  in the tier2 container.
        \returns True if lhs receiveTime is equal to rhs receiveTime */
    bool operator == (const Tier2Entry &rhs) {
        return (this->recvTime == rhs.recvTime);
    }
    
    bool operator < (const Tier2Entry &rhs) {
        return (this->recvTime > rhs.recvTime);
    }
    
    Time getRecvTime() const {
        return recvTime;
    }
    
    AgentID getAgentID() const {
        return agentID;
    }
};

class EventComp {
public:
    inline bool operator() (const Tier2Entry lhs, const Tier2Entry rhs) const {
        return (lhs.getRecvTime() > rhs.getRecvTime() );
    }
};

/** The Tier2 of type BinaryHeap.
    This is a heap of Tier2Entry objects that can store concurrent events.
*/
//typedef muse::BinaryHeap<Tier2Entry, EventComp> Tier2;

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
    
    /** Determine if the event queue is empty.
        
        This method implements the base class API to report if any
        events are pending to be processed in the event queue.

        \return This method returns true if the event queue of the
        top-agent is logically empty.
    */    
    virtual bool empty() {
        return (agentList.empty() || top()->eventPQ->empty());
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
        the least-time-stamp is at the top of the heap.

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

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.  Currently, this value is
        not used.

        \param[in] event The list of events to be enqueued.  The
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
    inline static  bool compare(const muse::Agent* const lhs,
                                const muse::Agent* const rhs) {
        return (lhs->getTopTime() >= rhs->getTopTime());
    }
    
    /** \brief compares the receive times of events 
    *  This will allow the sorting of events in the EventContainer
    *  by the event receive time 
    \returns True if lhs receiveTime is less than rhs receiveTime */
    inline static bool compareEvents(const muse::Event* lhs, const muse::Event* rhs) {
        return (lhs->getReceiveTime() < rhs->getReceiveTime());
    }
    
    /** \brief compares agent id and receive times of events
    *  This will allow the filtering of unique events in the EventContainer
    *  using std::unique
    \returns True if lhs agentID is equal to rhs agentID and lhs
     receiveTime is equal to rhs receiveTime*/
    inline static bool compareEventAttributes(const muse::Event* lhs,
                                              const muse::Event* rhs) {
         return (lhs->getReceiverAgentID() == rhs->getReceiverAgentID()
           && lhs->getReceiveTime() == rhs->getReceiveTime());
    }
    
    /** Convenience method to obtain the top-most or front agent.

        This method can be used to obtain a pointer to the top/front
        agent -- that is, the agent with the lowest timestamp event to
        be scheduled next.

        \return A pointer to the top-most agent in this heap.
    */
    muse::Agent* top() {
        return agentList.front();
    }

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
    
private:
    /** The backing storage for events managed by this class.

        This vector contains the list of agents being managed by the
        class.  The agents in the vector are stored and maintained as
        a heap.  The heap is created and managed using standard C++
        algorithms, namely: \c std::make_heap, \c std::push_heap, and
        \c std::pop_heap.
    */
    std::vector<muse::Agent*> agentList;
    
    /** Storage for concurrent events.
     * 
     *  This vector contains the list of Tier2Entry objects that store the 
     *  concurrent events. 
     */
    std::vector<Tier2Entry> tier2;     

};

END_NAMESPACE(muse)

#endif
