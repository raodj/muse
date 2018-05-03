#ifndef MUSE_EVENT_QUEUE_H
#define MUSE_EVENT_QUEUE_H

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

#include "DataTypes.h"
#include "EventRecycler.h"

BEGIN_NAMESPACE(muse)

// forward declare (muse namespace)
class Agent;

/** Base class for the different queues used by the different
    Schedulers.

    <p>The primary objective of an EventQueue is to act as a fast
    priority queue that provides access to events to be scheduled in a
    Time Warp simulation.  The priority (or sorting order) of events
    is based both on the receive-time of the event and the agent to
    which the event is scheduled.  The objective is to obtain all
    events scheduled to a given agent at a given receive-time in one
    batch/set.</p>

    <p>The EventQueue class serves as an abstract base class that
    defines the interface available to the Scheduler class hierarchy.
    The EventQueue class cannot be directly instantiated.  Instead one
    of the derived classes must be instantiated and used.</p>
*/
class EventQueue {
    friend class BinaryHeapWrapper;
public:
    /** Add/register an agent with the event queue.

        This method must be invoked to add/register each agent that is
        present on the local Simulator/process (.i.e, do not register
        all agents that are present on other compute nodes/processes).
        This method must be called prior to commencement of scheduling
        events via this event queue.

        \param[in,out] agent A pointer to the agent to be registered.
        The pointer is internally used by derived classes to manage
        corss references etc.  Consequently, the object pointed must
        not be deleted until the event queue operations are completed.

        \return This method returns an internal cross-reference to be
        stored in an agent.  This cross-reference is used by derived
        classes to optimize event management operations.
    */
    virtual void* addAgent(muse::Agent* agent) = 0;

    /** Remove/unregister an agent from the event queue.

        This method is invoked just before an agent is removed from
        the simulation.  Typically this method is invoked to inform
        the event queue that the data structures associated with the
        agent can be removed.

        \param[in,out] agent A pointer to the agent to be removed.
        The pointer is internally used by derived classes to manage
        cross references etc.  Consequently, the object pointed must
        not be deleted by this method.
    */
    virtual void removeAgent(muse::Agent* agent) = 0;
    
    /** Determine if the event queue is empty.

        This method must be implemented by the derived classes to
        report if any events are pending to be processed in the event
        queue.

        \return This method returns true if the event queue is
        logically empty.
    */
    virtual bool empty() = 0;

    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method can be used to obtain a pointer to the highest
        priority event in this event queue, without de-queuing the
        event.

        \note The event returned by this method is not dequeued.
        
        \return A pointer to the next event to be processed.  If the
        queue is empty then this method returns NULL.
    */
    virtual muse::Event* front() = 0;
    
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
    virtual void dequeueNextAgentEvents(muse::EventContainer& events) = 0;

    /** Enqueue a new event.

        This method must be used to enqueue/add an event to this event
        queue.  Once added the reference count on the event is
        increased.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.
        
        \param[in] event The event to be enqueued.  This parameter can
        never be NULL.
    */
    virtual void enqueue(muse::Agent* agent, muse::Event* event) = 0;

    /** Enqueue a batch of events.

        This method can be used to enqueue/add a batch of events to
        this event queue.  Note that the event reference count is not
        changed by this method.  This method provides a convenient
        approach to enqueue a batch of events, particularly after a
        rollback.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.

        \param[in] event The list of events to be enqueued.  The
        reference counts of the events in the container remains
        unmodified.  The list of events become part of the event
        queue.
    */
    virtual void enqueue(muse::Agent* agent, muse::EventContainer& events) = 0;

    /** Dequeue all events sent by an agent after a given time.

        This method can be used to remove/erase events sent by a given
        agent after a given simulation time.  This API is needed to
        cancel events during a rollback.

        \param[in] dest The agent whose currently secheduled events
        are to be checked and cleaned-up.  This agent must be a valid
        agent that has been registered/added to this event queue.  The
        pointer cannot be NULL.
        
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
                           const muse::Time sentTime) = 0;

    /** Print full contents of scheduler queue to given output stream.

        This is a convenience method that is used primarily for
        troubleshooting purposes.  This method is expected to print
        the complete set of events in the event queue to the given
        output stream.  The format of the output is dependent of the
        final derived class that is overridin this event.

        \param[out] os The output stream to which the contents of the
        queue are to be written.
    */
    virtual void prettyPrint(std::ostream& os) const = 0;

    /** Method to report aggregate statistics.

        This method is invoked at the end of simulation after all
        agents on this rank have been finalized.  This method can
        report any aggregate statistics from the event queue. The
        statistics must be written to the supplied output stream.

        \param[out] os The output stream to which the statistics are
        to be written.
    */
    virtual void reportStats(std::ostream& os) = 0;
    
    /** The virtual destructor.

        The destructor does not have any specific task as the base
        class does not manage or perform any special operations.
    */
    virtual ~EventQueue() {}

    /** Obtain the human-readable identifier/name.

        This string is an identifier that can be used to identify the
        final derived class.  It is handy when reporting errors or
        issues.

        \return This method returns the identifier set by the derived
        class during instantiation.
     */
    const std::string& getName() const { return name; }

    /** Fixup the heap after a value at a given index position has
        been updated.

        This method can be used to fix up the heap after the value at
        the given index position has been updated.  This method moves
        the value at the given index position up or down the heap to
        restore the heap properly.

        \note The time complexity of this method is O(log n).

        \param[in] vec The vector whose entry is to be updated.  This
        vector is the backing vector in which all values, except the
        one at index position, satisfy the heap property.

        \param[in] index The index position in the backing vector
        whose value is to be appropriately placed to restore the heap
        property.

        \param[in] comp A binary predicate comparator to be used to
        compare elements in the heap.

        \return This method returns the index position in the vector
        where the element was finally placed in the heap.
    */
    template<typename Val, typename Compare>
    static size_t fixHeap(std::vector<Val>& vec, size_t index, Compare comp) {
        if (index >= vec.size()) {
            return index;  // No further operation can be done!
        }
        const Val value = vec[index];  // A convenience copy!
        size_t parent   = (index > 0 ? (index - 1) / 2 : 0);
    
        if (comp(vec[parent], value)) {
            // The value need to move towards the root.
            while ((index > 0) && comp(vec[parent], value)) {
                std::swap(vec[index], vec[parent]);
                index  = parent;
                parent = (index - 1) / 2;
            }
        } else {
            // The value needs to move towards the leaves
            const size_t len = vec.size();
            size_t child1 = 2 * (index + 1) - 1;
            size_t child2 = std::min(vec.size() - 1, child1 + 1);
            while ((child1 < len) && (comp(value, vec[child1]) ||
                                      comp(value, vec[child2]))) {
                if (comp(vec[child1], vec[child2])) {
                    child1 = child2;
                }
                std::swap(vec[index], vec[child1]);
                index  = child1;
                child1 = 2 * (index + 1) - 1;
                child2 = std::min(vec.size() - 1, child1 + 1);
            }
        }
        return index;
    }

    /** Method to set if events are directly being shared between
        threads on this process.

        This method is invoked from the derived multi-threaded
        Simulation class(es) to set a flag to indicate if events are
        being shared between threads on this process.

        \note This method must be called before events are added to
        this event queue.  Calling this method after events are added
        results in undefined behaviors.
     */
    static void setUsingSharedEvents(bool sharedEvents);
    
protected:
    /** The constructor for this EventQueue.

        The constructor has been made private to ensure that this
        class is never directly instantiated.  Instead, one of the
        derived EventQueue classes must be instsantiated and used.

        \param[in] name A human-readable identifier/name to be set
        with this qeueue.
    */
    EventQueue(const std::string& name) : name(name) {}

    /** \brief Convenience wrapper to increase the internal reference
        counter for event -- only for single threaded model.

        This is just a convenience method that can be used by all the
        derived queue implementations to decrease reference on an
        event indicating that is can be recycled (or deleted).

        \param[in,out] event The event whose reference count is to be
        decreased.  This parameter cannot be NULL/nullptr.
        
        \see EventRecycler::decreaseReference
    */
    inline static void decreaseReference(muse::Event* const event) {
        EventRecycler::decreaseInputRefCount(usingSharedEvents, event);
    }

    /** \brief Convenience wrapper to increase the internal reference
        counter for event -- only for single threaded model.

        This is just a convenience method that can be used by all the
        derived queue implementations to increase reference on an
        event indicating that is is being used and it \b cannot be
        recycled (or deleted).

        \param[in,out] event The event whose reference count is to be
        decreased.  This parameter cannot be NULL/nullptr.
        
        \see EventRecycler::increaseReference
    */
    static inline void increaseReference(muse::Event* const event) {
        EventRecycler::increaseInputRefCount(usingSharedEvents, event);
    }

    /** Flag to indicate if shared events are used -- that is events
        are directly shared between two threads on this process.

        This flag is set via call to setUsingSharedEvents from the
        multi-threaded Simulation derived classes.  The flag is used
        in the increaseReference and decreaseReference methods in this
        class to use unified/split reference counters.

        \note This flag is intentionally static so that the static
        methods in this class can use it.
    */
    static bool usingSharedEvents;

private:
    /** A human-readable identifier/name associated with this queue.

        This instance variable is set in the constructor and is never
        changed during the life time of this class.  This string is an
        identifier that can be used to identify the final derived
        class.
    */
    std::string name;
};

END_NAMESPACE(muse)

#endif
