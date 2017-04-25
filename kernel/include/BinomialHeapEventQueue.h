#ifndef BINOMIAL_HEAP_EVENT_QUEUE_H
#define BINOMIAL_HEAP_EVENT_QUEUE_H

#include <vector>
#include "EventQueue.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wempty-body"
#endif

#include <boost/heap/binomial_heap.hpp>

#ifdef __GNUC__
#pragma GCC diagnostic warning "-Wempty-body"
#endif


BEGIN_NAMESPACE(muse)

struct EventCompare{
    bool operator() (const muse::Event* lhs,
                               const muse::Event* rhs) const {
         return ((lhs->getReceiveTime() > rhs->getReceiveTime()) ||
                ((lhs->getReceiveTime() == rhs->getReceiveTime() &&
                  (lhs->getReceiverAgentID() > rhs->getReceiverAgentID()))));
    } 
};

using Handle =  boost::heap::binomial_heap<muse::Event*, boost::heap::compare<EventCompare>>::handle_type;

/** A standard heap-based event queue for managing events.

    <p>This class provides a very simple binomial heap-based event queue
    for managing events for simulation.  This class uses the boost library
    implementation of the binomial heap to manage a binomial heap of events.</p> 
*/
class BinomialHeapEventQueue : public EventQueue {
public:
    /** The constructor for the BinomialHeapEventQueue.

        The default (and only) constructor for this class.  The
        constructor does not have any specific task to perform other
        than set a suitable identifier in the base class.
    */    
    BinomialHeapEventQueue();

    /** The destructor.

        The destructor does not have any special tasks to perform.
        All the dynamic memory management is handled by the standard
        containers (namely std::vector) that is used internally by
        this class.
    */
    ~BinomialHeapEventQueue();

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

    /** Remove/unregister an agent with the event queue.

        <p>This method implements the corresponding API method in the
        class.  Refer to the API documentation in the base class for
        intended functionality.</p>

        <p>This method removes all events scheduled for the specified
        agent in its internal data structures.</p>
        
        \param[in,out] agent A pointer to the agent whose events are
        to be removed from the heap managed by this class.
    */
    void removeAgent(muse::Agent* agent) override;
    
    /** Determine if the event queue is empty.
        
        This method implements the base class API to report if any
        events are pending to be processed in the event queue.
        
        \return This method returns true if the event queue is
        logically empty.
    */    
    virtual bool empty() { return binomialHeap.empty();  }

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
    
private:
    
    /** The priority queue for managing events.

        The binomial heap contains the list of events being managed by the
        class. The binomial heap is created and managed using boost libraries.
    */
    boost::heap::binomial_heap<muse::Event*, boost::heap::compare<EventCompare> > binomialHeap;
    
    /** A vector that is used to store the handle of an object.
      
        The vector is used to keep track of the handle of each object
        inserted into the binomial heap. The handle allows the elements of the 
        heap to be manipulated (i.e. increase/decrease priority and deletion).
     
     */
    std::vector<Handle> handles;

    /** This instance variable tracks to maximum number of events in
        the eventList.

        This instance variable is used to track the maximum number of
        events in the event list.  This value is reported (typically
        at the end) when the reportStats() method is invoked.
    */
    size_t maxQsize;
};

END_NAMESPACE(muse)

#endif /* BINOMIAL_HEAP_EVENT_QUEUE_H */

