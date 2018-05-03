#ifndef MUSE_EVENT_QUEUE_MT_H
#define MUSE_EVENT_QUEUE_MT_H

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

#include "EventQueue.h"

BEGIN_NAMESPACE(muse)

/** Base class for the different queues used by the different
    Schedulers in a Multi-Threaded Context

    <p>The primary objective of an EventQueueMT is to act as a fast
    priority queue that provides access to events to be scheduled in a
    Time Warp simulation.  The priority (or sorting order) of events
    is based both on the receive-time of the event and the agent to
    which the event is scheduled.  The objective is to obtain all
    events scheduled to a given agent at a given receive-time in one
    batch/set.</p>

    <p>The EventQueueMT class serves as an abstract base class that
    defines the interface available to the Scheduler class hierarchy.
    The EventQueueMT class cannot be directly instantiated.  Instead one
    of the derived classes must be instantiated and used.</p>
*/
class EventQueueMT : public EventQueue {
public:
    
    /**
     * These methods are not valid in thread safe operation, and are thus
     * overriden to an abort command in the MT version of the queue base class
     */
    muse::Event* front() override { std::cout << "Illegal Method"; abort();}
    
    /**
     * These methods are not valid in thread safe operation, and are thus
     * overriden to an abort command in the MT version of the queue base class
     */
    bool empty() override { std::cout << "Illegal Method"; abort();}
    
    /**
     * In a thread safe pending event priority queue, agent must be "locked"
     * for dequeue operations prior to processing any events.
     * 
     * The process for this is...
     *     1. popNextAgent      - get exclusive access to an agent
     *     2. dequeueNextEvents - actually process the events
     *     3. pushAgent         - put the agent back for future processing
     * 
     * Attempting to use this method will cause the sim to abort
     */
    virtual void dequeueNextAgentEvents(muse::EventContainer& events) override {
        UNUSED_PARAM(events);
        std::cout << "Illegal Method"; abort();
    }
    
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
    
    /**
     * This method pops the next agent off the top level queue, thus providing
     * exclusive dequeue rights to this thread for that agent.
     * 
     * This agent must be returned back into the queue when processing of it is
     * done via EventQueue::returnAgent(). 
     * 
     * @return the next agent to be processed by the sim
     */
    virtual muse::Agent* popNextAgent() = 0;
    
    /** This method gets the next set of events for an agent. Note that
        the agent must be popped first, as the agent provided must be exlusively
        held by this thread for dequeue purposes.

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
    virtual void dequeueNextEvents(muse::Agent *agent, 
            muse::EventContainer& events) = 0;
    
    /**
     * Puts the agent back into the top level queue for future processing
     * 
     * @param agent
     */
    virtual void pushAgent(muse::Agent *agent) = 0;

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
    virtual ~EventQueueMT() {}
    
protected:
    /** The constructor for this EventQueueMT.

        The constructor has been made private to ensure that this
        class is never directly instantiated.  Instead, one of the
        derived EventQueueMT classes must be instsantiated and used.

        \param[in] name A human-readable identifier/name to be set
        with this qeueue.
    */
    EventQueueMT(const std::string& name) : EventQueue(name) {}

};

END_NAMESPACE(muse)

#endif
