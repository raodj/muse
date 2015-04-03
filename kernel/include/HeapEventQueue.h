#ifndef HEAP_EVENT_QUEUE_H
#define HEAP_EVENT_QUEUE_H

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

BEGIN_NAMESPACE(muse)

/** A standard heap-based event queue for managing events.

    <p>This class provides a very simple (good candidate for base-case
    comparisons) heap-based event queue for managing events for
    simulation.  This class uses standard C++ algorithms (such as: \c
    std::make_heap, \c std::push_heap, \c std::pop_heap) to manage a
    heap of events.  The events are stored in a backing
    std::vector.</p>
    
    <p>The behavior of the data structure exposed by this class is
    very close to the \c std::priority_queue.  The differences between
    the two are subtle in the sense that this class provides a few
    extra features, namely:

    <ol>

    <li>Explicit control over addition and removal of events from the
    heap.</li>

    <li>Methods for bulk removal of events necessary for efficient
    event managment during parallel simulation (to cancel events after
    a rollback).

    </ol>
    
    </p>
*/
class HeapEventQueue : public EventQueue {
public:
    /** The constructor for the HeapEventQueue.

        The default (and only) constructor for this class.  The
        constructor does not have any specific task to perform other
        than set a suitable identifier in the base class.
    */    
    HeapEventQueue();

    /** The destructor.

        The destructor does not have any special tasks to perform.
        All the dynamic memory management is handled by the standard
        containers (namely std::vector) that is used internally by
        this class.
    */
    ~HeapEventQueue();

    /** Add/register an agent with the event queue.

        <p>This method implements the corresponding API method in the
        class.  Refer to the API documentation in the base class for
        intended functionality.</p>

        <p>This class does not utilize the agent registration
        information and consequently this method does not perform any
        special action.</p>

        \param[in,out] agent A pointer to the agent to be registered.
        This value is not used.

        \return This method returns NULL as its internal
        cross-reference to be stored in an agent.
    */    
    virtual void* addAgent(muse::Agent* agent);
    
    /** Determine if the event queue is empty.
        
        This method implements the base class API to report if any
        events are pending to be processed in the event queue.
        
        \return This method returns true if the event queue is
        logically empty.
    */    
    virtual bool empty() { return eventList.empty();  }

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

        In MUSE agents are scheduled to process all events at a given
        simulation time.  The next concurrent events (i.e., events
        with the same receive time) with the lowest time stamp are to
        be placed in the supplied event container.  The event
        container is then passed to the corresponding agent for
        further processing.

        \param[out] events The event container in which the next set
        of concurrent events are to be placed.  Note that the order of
        concurrent events in the event container is unspecified.
    */
    virtual void dequeueNextAgentEvents(muse::EventContainer& events);

    /** Enqueue a new event.

        This method must be used to enqueue/add an event to this event
        queue.  Once added the reference count on the event is
        increased.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.  Currently, this method
        does not really use this value.
        
        \param[in] event The event to be enqueued.  This parameter can
        never be NULL.
    */
    virtual void enqueue(muse::Agent* agent, muse::Event* event);

    /** Enqueue a batch of events.

        This method can be used to enqueue/add a batch of events to
        this event queue.  Once added the reference count on each one
        of the events is increased.  This method provides a convenient
        approach to enqueue a batch of events, particularly after a
        rollback.

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
        during a rollback.

        \param[in] dest The agent whose currently secheduled events
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
    /** Convenience method to remove the front event.

        This is an internal convenience method that is used to remove
        the front (i.e., event with lowest timestamp) event from this
        queue.  This method uses std::pop_heap to remove the event,
        thereby ensuring that the heap property is maintained.

        \return If the event queue is empty, then this method return
        NULL.  Otherwise it removes the lowest timestamp event from
        the queue and returns it.
    */
    muse::Event* pop_front();

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
    static inline bool compare(const muse::Event* const lhs,
                               const muse::Event* const rhs) {
        return ((lhs->getReceiveTime() > rhs->getReceiveTime()) ||
                ((lhs->getReceiveTime() == rhs->getReceiveTime() &&
                  (lhs->getReceiverAgentID() > rhs->getReceiverAgentID()))));
    }

private:
    /** The backing storage for events managed by this class.

        This vector contains the list of events being managed by the
        class.  The events in the vector are stored and maintained as
        a heap.  The heap is created and managed using standard C++
        algorithms, namely: \c std::make_heap, \c std::push_heap, and
        \c std::pop_heap.
    */
    std::vector<Event*> eventList;

    /** This instance variable tracks to maximum number of events in
        the eventList.

        This instance variable is used to track the maximum number of
        events in the event list.  This value is reported (typically
        at the end) when the reportStats() method is invoked.
    */
    size_t maxQsize;
};

END_NAMESPACE(muse)

#endif
