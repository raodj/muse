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

#include <unordered_map>
#include <stack>
#include "config.h"
#include "Event.h"
#include "NumaMemoryManager.h"

BEGIN_NAMESPACE(muse);

// Forward declaration for some of the classes.
class MultiThreadedCommunicator;
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
    friend class GVTMessage;
    friend class MultiThreadedSimulation;
    friend class MultiThreadedSimulationManager;
    friend class MultiThreadedShmSimulation;
    friend class MultiThreadedShmSimulationManager;
    friend class OCLAgent;
public:
    /** The default NUMA settings for memory management.

        These enumerations are primarily applicable only when
        multi-threaded simulations are used.  The default setting is
        always NUMA_NONE. Since these values relate to
        multi-threading, the actual setting to be used to setup in
        various multi-threading simulators during start-up.
    */
    enum NumaSetting {
        /** No NUMA usage and use default new/delete operators */
        NUMA_NONE,
        /** Default NUMA allocations based on sender's agent ID. By
            default memory is allocated on the sender-thread's NUMA
            node.  Typically used when <u>shared events are not
            used</u>.
        */
        NUMA_SENDER,
        /** Default NUMA allocations based on receiver agent ID. By
            default memory is allocated on the receiving-thread's NUMA
            node.  Typically used when <u>event shared is enabled.</u>.
        */
        NUMA_RECEIVER
    };
    
    /** Convenience interface method to allocate flat memory for a
        given event.

        This method is a convenience method to streamline memory
        recycling operations for events with default or NUMA-aware
        memory management.  This method is uses a compile-time macro
        to call the default or NUMA-aware helper methods in this
        class.  The default memory management also permits
        enabling/disabling event recyling via the RECYCLE_EVENTS
        compile-time macro.  However, the NUMA-aware memory management
        (if available and enabled at runtime) requires/uses event
        recycling and the RECYCLE_EVENTS macro must be nabled.

        \note This method is static and consequently thread safe and
        can be simultaneously called from multiple threads.
        
        \param[in] size The size of the flat buffer to be allocated
        for storing event information.

        \param[in] receiver The destination agent to which the event
        is to be sent.  This value is used by the NUMA-aware memory
        allocator to manage memory.  If NUMA is not being used, then
        this value is not used.  If this value is -1, then memory is
        allocated on the local NUMA node of the thread calling this
        method.
        
        \return A pointer to a valid/flat buffer. This method always
        returns a valid event pointer.
    */    
    static char* allocate(const int size, const muse::AgentID receiver) {
#if USE_NUMA == 0
        return allocateDefault(size, receiver);
#else
        return allocateNuma(numaSetting, size, receiver);
#endif
    }

    /** This method is the dual/converse of allocate to recycle events
        if recycling is enabled.

        This method is a convenience method that provide a
        compile-time option to use NUMA-aware memory management, if
        NUMA-libraries are available.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        \param[in] event The event to be deallocated/recycled.
    */
    static void deallocate(muse::Event* event) {
#if USE_NUMA == 0
        deallocateDefault(event);
#else
        deallocateNuma(event);
#endif
    }

    /** Helper method to clear out all events in the recycler.

        <p>This method is typically called at the end of the
        simulation from Simulation::finalize() or
        MultiThreadedSimulation::simulate() method.  The overall
        functionality is to deletes all events in the thread-local
        EventRecycler and empty's it.</p>

        <p>This method uses a compile-time flag to either call
        deleteRecycledEventsDefault() (if NUMA is not used) or
        deleteRecycledEventsNuma() (when NUMA is being used)</p>
    */
    static void deleteRecycledEvents() {
#if USE_NUMA == 0
        deleteRecycledEventsDefault();
#else
        deleteRecycledEventsNuma();
#endif
    }
    
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

    /** Method to print internal statistics.

        This method returns a string containing a human-readable form
        of the internal statistics maintained by the default and NUMA
        memory manager on this thread.

        \note This method reports thread-specific information.

        \return A string containing statistics informaton that can be
        readily printed.
    */    
    static std::string getStats();

protected:
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

        \param[in] receiver The destination agent to which the event
        is to be sent.  This value is used by the NUMA-aware memory
        allocator to manage memory.  If NUMA is not being used, then
        this value is not used.  If this value is -1, then memory is
        allocated on the local NUMA node of the thread calling this
        method.
        
        \return A pointer to a valid/flat buffer. This method always
        returns a valid event pointer.
    */
    static char* allocateDefault(const int size, const muse::AgentID receiver);
    
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
    static void deallocateDefault(char* buffer, const int size);

    /** This method is the dual/converse of allocate to recycle events
        if recycling is enabled.

        This method is a convenience method that calls deallocate
        method with necessary parameters.  This method streamlines
        calls to the deallocate method in various spots in the
        EventRecycler source.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        \param[in] event The event to be deallocated/recycled.
    */
    static void deallocateDefault(muse::Event* event) {
        // Free-up or recycle the memory for this event.
#ifdef RECYCLE_EVENTS
        // Save event size (as destructor call below can change it)
        const int eventSize = event->getEventSize();
        // Manually call event destructor
        event->~Event();
        // Recycle the buffer
        deallocateDefault(reinterpret_cast<char*>(event), eventSize);
#else 
        // Manually call event destructor
        event->~Event();       
        // Avoid 1 extra call to getEventSize() in this situation
        deallocateDefault(reinterpret_cast<char*>(event), 0);
#endif
    }

#if USE_NUMA == 1
    /** Convenience method for NUMA-aware allocatation for events.

        This method is a convenience method to streamline NUMA-aweare
        memory operations.  This method requires memory recycling for
        events to be enabled using compile-time macro RECYCLE_EVENTS.
        If default NUMA allocation is NUMA_RECEIVER then the
        receiver's ID is used to determine NUMA node on which memory
        is to be allocated.  Otherwise the specified thread's ID is
        used.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        \param[in] numa The NUMA setting to be used when allocating
        memory.
        
        \param[in] size The size of the flat buffer to be allocated
        for storing event information.

        \param[in] receiver The destination agent to which the event
        is to be sent.  This value is used by the NUMA-aware memory
        allocator to manage memory.  If NUMA is not being used, then
        this value is not used.  If this value is -1, then memory is
        allocated on the local NUMA node of the thread calling this
        method.
        
        \return A pointer to a valid/flat buffer. This method always
        returns a valid event pointer.
    */
    static char* allocateNuma(const EventRecycler::NumaSetting numaMode,
                              const int size, const muse::AgentID receiver);

    /** Convenience method for NUMA-aware allocatation for GVT
        messages to be sent to a differen thread.

        This method is a convenience method to streamline NUMA-aweare
        memory operations.  This method requires memory recycling for
        events to be enabled using compile-time macro RECYCLE_EVENTS.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        \param[in] size The size of the flat buffer to be allocated
        for storing event information.

        \param[in] destThreadID The destination thread to which the
        event is to be sent. If NUMA is not being used, then this
        value is not used.  If this value is -1, then memory is
        allocated on the local NUMA node of the thread calling this
        method.
        
        \return A pointer to a valid/flat buffer. This method always
        returns a valid event pointer.
    */
    static char* allocateNuma(const int size, const int destThreadID);
    
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
    static void deallocateNuma(char* buffer, const int size);

    /** This method is the dual/converse of allocate to recycle events
        if recycling is enabled.

        This method is a convenience method that calls deallocate
        method with necessary parameters.  This method streamlines
        calls to the deallocate method in various spots in the
        EventRecycler source.

        \note This method is thread safe and can be simultaneously
        called from multiple threads.
        
        \param[in] event The event to be deallocated/recycled.
    */
    static void deallocateNuma(muse::Event* event) {
        if (numaSetting != NUMA_NONE) {
            // Save event size (as destructor call below can change it)
            const int eventSize = event->getEventSize();        
            // Manually call event destructor
            event->~Event();
            // Free-up or recycle the memory for this event.
            deallocateNuma(reinterpret_cast<char*>(event), eventSize);
        } else {
            // NUMA use is disabled at runtime.
            deallocateDefault(event);
        }
    }

    /** Helper method to clear out all events in the recycler.

        This method is typically called at the end of the simulation
        from Simulation::finalize() method.  This method deletes all
        events in the EventRecycler and empty's it.
    */
    static void deleteRecycledEventsNuma();
    
#endif

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

    /** Setup NUMA-aware memory management for events.

        This method is invoked from
        MultiThreadedSimulationManager::initialize() to setup the
        NUMA-aware memory manager for use in multi-threaded simulation.

        \note This method must be called only from the main thread.
        
        \note It is important to call startNUMA at the beginning of
        each thread.
        
        \param[in] mtc The communicator that provides agent-to-thread
        mappings used to determine the destination thread for events.

        \param[in] numaIDofThread The list of local NUMA-nodes for
        each thread.  This list provides the nearest NUMA-node for
        each thread so that memory is allocated on that NUMA node.

        \param[in] numa The type of NUMA operation to be used by
        default when allocating memory for events.
    */
    static void setupNUMA(const MultiThreadedCommunicator* mtc,
                          const std::vector<int>& numaIDofThread,
                          NumaSetting numa = EventRecycler::NUMA_NONE);

    /** Per-thread method to start thread-local NUMA memory manager.

        This method must be called at the beginning of each thread to
        enable NUMA memory management for each thread.

        \note Unlike the setupNUMA method, this method must be called
        from each thread.

        \param[in] blockSize The size of NUMA memory that should be
        allocated whenever additional memory is needed.
    */
    static void startNUMA(const int blockSize = 65536);
    
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
 
    /** The ID/index (zero-based) of the thread associated with this
        event recycler.

        This instance variable tracks the ID/index (zero-based) of the
        thread that is logically operating with this event recycler.
        This value is initialized to 0.  It is set when in the
        MultiThreadedSimulation::simulate() method, which is the main
        thread method.
    */
    thread_local static char threadID;

    /** Flag to indicate the type of NUMA-aware memory management to
        be used, including if NUMA is enabled/disabled.

        This setting is used to determine if NUMA-aware memory
        management has been enabled/disabled, and if enabled, the type
        of default NUMA-awareness to use.  The NUMA is usage is
        typically configured (actually disabled) via command-line
        argument '--no-numa' supported in multi-threaded simulations.
        In non-multi-threaded modes (and by default), this value is
        set to \c NUMA_NONE.
    */
    static NumaSetting numaSetting;

    /** List of NUMA node IDs for each thread to be used by NUMA-aware
        memory manager.

        This is a static (i.e., shared by all threads) list that
        contains the NUMA-node ID (0, 1, 2, ...) for each thread.  The
        zero-based threadID is used as the index into this vector to
        provide fast mapping between threadID and NUMA-node ID.  Note
        that many threads can have have same NUMA-node ID based on the
        number of cores on each CPU.  This list is setup in the
        setupNUMA method in this class.
    */
    static std::vector<int> numaIDofThread;

    /** The communicator that provides agent-to-thread mapping.

        This communicator is used only for NUMA-aware memory
        management.  By default it is NULL.  This pointer is set in
        the setupNUMA method.
    */
    static const MultiThreadedCommunicator* mtc;
    
    /** Helper method to clear out all events in the recycler.

        This method is typically called at the end of the simulation
        from Simulation::finalize() method.  This method deletes all
        events in the EventRecycler and empty's it.
    */
    static void deleteRecycledEventsDefault();

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

#if USE_NUMA == 1
    /** A thread-local NUMA memory manager for managing memory in a
        NUMA-aware manner.

        This is a thread local numa memory manager that performs the
        core tasks of managing memory in a NUMA-aware fashion.
    */
    thread_local static NumaMemoryManager numaMemMgr;
#endif

    /** The number of method calls to the allocateDefault method in
        this class.

        This counter tracks the number of calls to the allocateDefault
        method.
    */
    thread_local static size_t allocCalls;

    /** The number of deallocationDefault method calls.

        This counter tracks the number of deallocateDefault calls.

        \note With events going across threads, the number of allocate
        vs. deallocate calls will not match-up.  However, the total
        number across multiple threads should all add-up.
    */
    thread_local static size_t deallocCalls;

    /** The number of times memory requests were satisfied from the
        default event recycler.

        This counter tracks the number of times an allocationDefault
        request was satisfied using a recycled memory.  Having a high
        hit rate is important for efficient NUMA utilization.
    */
    thread_local static size_t recycleHits;

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
