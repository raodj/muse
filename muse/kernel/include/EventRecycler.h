#ifndef MUSE_EVENT_RECYCLER_H
#define MUSE_EVENT_RECYCLER_H

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

#include "Event.h"
#include <unordered_map>
#include <stack>

BEGIN_NAMESPACE(muse);

// Forward declaration for muse::Event class
class Event;

/** An unordered map to store free events of different sizes.

    The key into this unordered map is the size of the event being
    recycled.  For each size a stack is maintained to return the
    most recently used event to improve cache performance.
 */
using RecycleMap = std::unordered_map<int, std::stack<char*>>;

/** A convenience class for enabling/disabling (at compile time) event
    recycling.
    
    <p>This a refactored utility class is a convenience base class for
    all events in a MUSE simulation.  Prior to Dec 2017, these methods
    were an integral part of of muse::Event class.  However, with
    increasing functionality it was deemed prudent to refactor these
    methods into its own class to streamline the code.</p>

    \note All methods in this class are intentionally designed to be
    static methods as they can be called from multiple threads.
    Furthermore, some of the methods are also invoked prior to
    instantiation of muse::Event objects.

    \see muse::Event
*/
class EventRecycler {
    friend class Agent;
    friend class EventQueue;
    friend class Simulation;
    friend class MultiThreadedSimulation;
    friend class MultiThreadedSimulationManager;
    friend class MultiThreadedShmSimulation;
    friend class MultiThreadedShmSimulationManager;
public:
    /** Convenience method to allocate flat memory for a given event.

        This method is a convenience method to streamline memory
        recycling operations for events.  This method allows memory
        recycling for events to be enabled/disabled using compile-time
        macro RECYCLE_EVENTS.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        <p>If recycling of events is disabled, this method always
        creates a flat array of characters using new[] operator and
        returns it.</p>

        <p>On the other hand, if recycling of events is enabled (via
        compiler flag RECYCLE_EVENTS), then this method operates as
        follows.  First it checks to see if EventRecycler has an entry
        of the specified size.  If so it returns it.  Otherwise it
        creates a new flat array of characters (using new[] operator)
        and returns it.</p>

        \param[in] size The size of the flat buffer to be allocated
        for storing event information.

        \return A pointer to a valid/flat buffer. This method always
        returns a valid event pointer.
    */
    static char* allocate(const int size);

    /** This method is the dual/converse of allocate to recycle events
        if recycling is enabled.

        This method is a convenience method to streamline memory
        recycling operations for events.  This method allows memory
        recycling for events to be enabled/disabled using compile-time
        macro RECYCLE_EVENTS.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        <p>If recycling of events is disabled, this method simply
        deletes the buffer using delete[] operator.</p>

        <p>On the other hand, if recycling of events is enabled (via
        compiler flag RECYCLE_EVENTS), then this method adds the
        buffer to the appropriate entry in the EventRecycler map in
        this class.</p>
        
        \param[in] buffer The event buffer previously obtained via
        call to the allocate method in this class.

        \param[in] size The size of the buffer (in bytes).  The size
        is used to recycle the buffer in future calls to allocate.
     */
    static void deallocate(char* buffer, const int size);
    
    /** Convenience method to get the reference count on events for
        troubleshooting/debugging purposes.

        \note This method is thread safe under the assumption that
        only the sending thread calls/uses this method.
        
        \return The reference count on events. For valid events this
        value is in the range 0 < referenceCount < 3.
    */
    static int getReferenceCount(const muse::Event* const event);

    /** Convenience method to get the input reference count on events
        for troubleshooting/debugging purposes.

        \note This method is thread safe under the assumption that
        only the receiving thread calls/uses this method.
        
        \return The input reference count on events. For valid events
        this value is in the range 0 < inputRefCount < 3.
    */
    static int getInputRefCount(const muse::Event* const event);
    
protected:
    /** \brief Decrease the internal reference counter for event --
        only for single threaded model.

        \note This method is thread safe under the assumption that
        only the sending thread calls/uses this method.
        
        Used for memory management.  This method should not be used by
        users. MUSE uses this to handle memory for you.

        \param[in,out] event The event whose reference counts are to
        be decreased.  If the reference counter reaches zero then the
        event is reclaimed/recycled.
    */
    static void decreaseReference(muse::Event* const event);
  
    /** \brief Increase the internal reference counter

        \note This method is thread safe under the assumption that
        only the sending thread calls/uses this method.
        
        Used for memory management.  This method should not be used by
        users. MUSE uses this to handle memory for you.
    */
    static inline void increaseReference(muse::Event* const event) {
        event->referenceCount++;
        // In 1 thread mode events can temporarily be in 3 spots:
        // input queue, output queue, scheduler queue.
        ASSERT(event->referenceCount < 4);
    }
    
    /** Helper method to increase input queue reference counter on
        events.  This method is used only by multi-threaded
        simulations where events are shared between threads.  When
        events are shared between 2 threads, each thread gets its own
        reference counter.  Events are deleted/recycled when both
        reference counters reach zero.

        \note This method is thread safe under the assumption that
        only the receiving thread calls/uses this method.
        
        \param[in,out] event The event on which the \c inputRefCount
        instance variable is to be incremented.
    */
    static void increaseInputRefCount(muse::Event* const event) {
        event->inputRefCount++;
        // Events can temporarily be in two spots: scheduler queue and
        // input queue of an agent.
        ASSERT(event->inputRefCount < 3);
    }

    /** Helper method to decrease input queue reference counter on
        events.  This method is used only by multi-threaded
        simulations where events are shared between threads.  When
        events are shared between 2 threads, each thread gets its own
        reference counters, namely: referenceCount (used by sending
        thread) and inputRefCount (used by receiving thread).  Events
        are deleted/recycled when both reference counters reach zero.

        \note This method is thread safe under the assumption that
        only the receiving thread calls/uses this method.  The method
        is designed to be used only in multi-threaded mode when
        directly sharing events between threads.  This method should
        be called only from the thread that received the event!

        \param[in,out] event The event on which the \c inputRefCount
        instance variable is to be decremented.  If the counter
        reaches zero, this event is recycled/deleted.
    */
    static void decreaseInputRefCount(muse::Event* const event) {
        ASSERT(event != NULL);
        ASSERT(event->inputRefCount > 0);
        event->inputRefCount--;
    }

    /** Helper method to input output queue reference counter (i.e.,
        Event::referenceCount) on events.  This is exactly the same
        call to increaseReference method but with a different name to
        provide a consistent naming convention for methods.

        \note This method is thread safe under the assumption that
        only the sending thread calls/uses this method.

        \param[in,out] event The event on which the \c referenceCount
        instance variable is to be increments.
    */
    static inline void increaseOutputRefCount(muse::Event* const event) {
        increaseReference(event);
    }
    
    /** Helper method to decrease output queue reference counter
        (i.e., Event::referenceCount) on events.  This method is used
        only by multi-threaded simulations where events are shared
        between threads.  When events are shared between 2 threads,
        each thread gets its own reference counters, namely:
        referenceCount (used by sending thread) and inputRefCount
        (used by receiving thread).  Events are deleted/recycled when
        both reference counters reach zero.

        \note This method is thread safe under the assumption that
        only the sending thread calls/uses this method.

        \param[in,out] event The event on which the \c referenceCount
        instance variable is to be decremented.  If the counter
        reaches zero, this event is marked to be recycled/deleted.
    */
    static void decreaseOutputRefCount(muse::Event* const event);    
    
    /** Convenience method to use unified or split reference counters
        in single-threaded or multi-threaded mode respectively.

        \param[in] input If this flag is true, it indicates events are
        being directly shared between multiple threads and split
        reference counters are to be used.  In this case, this method
        calls teh increaseInputRefCount method.  Otherwise (i.e.,
        input flag is false) this method calls the increaseReference
        method.

        \param[in,out] event The event whose reference counters are to
        be suitably updated.
    */
    static inline void increaseInputRefCount(const bool input,
                                             muse::Event* const event) {
        // Depending on flag increment input/receiver reference count
        // or the shared reference count (used in single threaded
        // mode).
        (input) ? increaseInputRefCount(event) : increaseReference(event);
    }

    /** Convenience method to use unified or split reference counters
        in single-threaded or multi-threaded mode respectively.

        \param[in] input If this flag is true, it indicates events are
        being directly shared between multiple threads and split
        reference counters are to be used.  In this case, this method
        calls the decreaseInputRefCount method.  Otherwise (i.e.,
        input flag is false) this method calls the decreaseReference
        method.

        \param[in,out] event The event whose reference counters are to
        be suitably updated.
    */
    static inline void decreaseInputRefCount(const bool input,
                                             muse::Event* const event) {
        // Depending on flag increment input/receiver reference count
        // or the shared reference count (used in single threaded
        // mode).
        (input) ? decreaseInputRefCount(event) : decreaseReference(event);
    }

    /** Convenience method to use unified or split reference counters
        in single-threaded or multi-threaded mode respectively.

        \param[in] input If this flag is true, it indicates events are
        being directly shared between multiple threads and split
        reference counters are to be used.  In this case, this method
        calls teh increaseInputRefCount method.  Otherwise (i.e.,
        input flag is false) this method calls the increaseReference
        method.

        \param[in,out] event The event whose reference counters are to
        be suitably updated.
    */
    static inline void increaseOutputRefCount(const bool input,
                                              muse::Event* const event) {
        // Depending on flag increment input/receiver reference count
        // or the shared reference count (used in single threaded
        // mode).
        (input) ? increaseOutputRefCount(event) : increaseReference(event);
    }

    /** Convenience method to use unified or split reference counters
        in single-threaded or multi-threaded mode respectively.

        \param[in] input If this flag is true, it indicates events are
        being directly shared between multiple threads and split
        reference counters are to be used.  In this case, this method
        calls the decreaseInputRefCount method.  Otherwise (i.e.,
        input flag is false) this method calls the decreaseReference
        method.

        \param[in,out] event The event whose reference counters are to
        be suitably updated.
    */
    static inline void decreaseOutputRefCount(const bool input,
                                              muse::Event* const event) {
        // Depending on flag increment input/receiver reference count
        // or the shared reference count (used in single threaded
        // mode).
        (input) ? decreaseOutputRefCount(event) : decreaseReference(event);
    }

    /** Convenience method to move all pending event deallocations to
        a given main list.

        This method is used by threads (other than the main thread) to
        move pending deallocations (if any) to the list of pending
        deallocations on the main thread, because -- Pending event
        deallocations on various threads cannot be fully reclaimed
        until all agents are finalized (and they relinquish references
        to events in their internal queues).  Consequently, at the end
        of the MultiThreadedSimulation::simulate() method, each thread
        (other than the main thread) adds its pending events to this
        list.  This list is finally cleaned-up in the main thread.

        \param[out] mainList The pending deallocation list on the main
        thread.
    */
    static void movePendingDeallocsTo(EventContainer& mainList);
    
private:
    /** An unordered map of stacks to recycle events of different
        sizes.

        This map is used only if RECYCLE_EVENTS macro has been enabled
        at compile time.  It has thread local storage so that each
        thread (in case multi-threaded mode is used) get's its own
        recycler thereby eliminating contention/locking.
        
        The key into this unordered map is the size of the event being
        recycled.  For each size a stack is maintained to return the
        most recently used event to improve cache performance.

        This map is used by the allocate and deallocate methods in
        this class.  Typically the Event::create method is the one
        that is used by applications to create an event.
    */
    thread_local static RecycleMap Recycler;

    /** A list of events pending to be deallocated/recycled.

        This list is used to hold a temporary list of events that are
        pending to be recycled.  This list is used only in
        multi-threaded scenarios when events are directly shared
        between two threads.  When sending thread is done with events
        (i.e. referenceCount == 0) but receiving thread is not
        (i.e. inputRefCount > 0), events are added to this list.
    */
    thread_local static EventContainer pendingDeallocs;
        
    /** Helper method to clear out all events in the recycler.

        This method is typically called at the end of the simulation
        from Simulation::finalize() method.  This method deletes all
        events in the EventRecycler and empty's it.
    */
    static void deleteRecycledEvents();

    /** Helper method to check and reclaim events in the
        pendingDeallocs list.

        This method is periodically called in the allocate method and
        the delteRecycledEvents method to clear out as many events as
        possible from the pendingDeallocs list.  This method iterates
        over the events in the pendingDeallocs list and deallocates
        events for with both reference counters are zero.
        
        \note This method turns out to be a bit time consuming as a
        large number of events can be pending and iterating over the
        list was taking ~65% of the runtime.  So this method is
        called from MultiThreadedSimulation::garbageCollect only when
        probability of free events is high.

        \see MultiThreadedSimulation::garbageCollect
    */
    static double processPendingDeallocs();
    
private:    
    /** The only constructor that is intentionally private to ensure
        that this class is never instantiated.
    */
    EventRecycler() {}

    /** The destructor that is intentionally private to ensure that
        this class is never instantiated.
    */
    ~EventRecycler() {}
};

END_NAMESPACE(muse);

#endif
