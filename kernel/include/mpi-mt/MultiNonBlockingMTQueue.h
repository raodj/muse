#ifndef MUSE_MULTI_NON_BLOCKING_MT_QUEUE_H
#define MUSE_MULTI_NON_BLOCKING_MT_QUEUE_H

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include <mutex>
#include "MTQueue.h"
#include <boost/lockfree/queue.hpp>

// A convenience type alias for accesing boost::lockfree::queue
using LockFreeQueue =
    boost::lockfree::queue<muse::Event*, boost::lockfree::fixed_sized<true>>;

BEGIN_NAMESPACE(muse);

/** A shared queue with multiple subqueues for exchanging events
    between threads.

    This class implements the features of the abstract MTQueue base
    class using one-or-more separate non-blocking sub-queues.  Each
    sub-queue list typically contains events/messages from other
    threads or those received over MPI.  It enables multiple threds to
    safely add/remove events in a MT-safe manner.  Key implementation
    aspects include:

    <ul>
    
    <li>It uses a list of boost::lockfree/queue for implementing each
    sub-queue</li>
    
    <li>Since all operations are lock free, this class does not use
    any mutex.</li>

    <li>The number of subqueues to be used.  Ideally this value must
    be a power of 2 as internally this data structure uses bit-wise
    operations to select a sub-queue based on source thread's ID.
    </li>

    <li>Note that sub-queue sizes do not change in this implementation
    to avoid issues with dynamic memory allocation.</li>
    
    </ul>
*/
class MultiNonBlockingMTQueue : public muse::MTQueue {
public:
    /** The default constructor.

        The default constructor currently reserves space in the
        underlying backing store to minimize inital reallocations.

        \param[in] numSubQueues The number of subqueues to be used.
        Ideally this value must be a power of 2 as internally this
        data structure uses bit-wise operations to select a sub-queue
        based on source thread's ID.
        
        \param[in] reserve The fixed number of entries to reserve for
        each subqueue.  Note that in this implementation subqueue
        sizes do not change.

        \param[in] batchSize When removing events, we don't want to
        spin in removeAll method consuming events as other threads
        produce them.  We want to get a batch of events and move on.
        Since LockQueueQueue does not have a size() method to tell
        number of elements in it, we will set an arbitrary size.
    */
    MultiNonBlockingMTQueue(int numSubQueues = 2, int reserve = 10240,
                            int batchSize = 25);
    
    /** Add an event into the queue.

        This method adds a given into the input queue.  This method
        must be implemented in a MT-safe (multi-threading safe) manner
        so that multiple threads can simultaneously call this method
        without experiencing race conditions.

        \param[in] srcThrIdx A logical zero-based index of the source
        thread that is sending the event.  This value could be -1 if
        the source thread is on a different process.  This value is
        not used by this method.

        \param[in] destThrIdx The index of the destination thread on
        the local process which will process the event. This value is
        not used by this method.
        
        \param[in] event Pointer to a MUSE event.  This pointer cannot
        be NULL.
    */
    virtual void add(int srcThrIdx, int destThrIdx,
                     muse::Event* event) override;

    /** Add a big batch of events into the queue.

        This method adds events from a given list into the input
        queue.  This method must be implemented in a MT-safe
        (multi-threading safe) manner so that multiple threads can
        simultaneously call this method without experiencing race
        conditions.

        \param[in] srcThrIdx A logical zero-based index of the source
        thread that is sending the event.  This value could be -1 if
        the source thread is on a different process.  This value is
        not used by this method.

        \param[in] destThrIdx The index of the destination thread on
        the local process which will process the event. This value is
        not used by this method.
        
        \param[in] eventList The list of events to be added to this
        queue.
    */
    virtual void add(int srcThrIdx, int destThrIdx,
                     EventContainer& eventList) override;

    /** Remove all the events in this queue into a given local list.

        This method performs bulk copying of all the events (if any)
        added to this queue into a separate list.  The container is
        passed-in by the caller giving the caller the ability to reuse
        containers (to minimize creation/resizing overheads).

        \param[out] eventList The container to which events are to be
        added.  Eisting entries in this list are left unmodified.

        \param[in] destThrIdx The index of the destination thread on
        the local process which will process the event.

        \param[in] numEvents The maximum number of events to be
        returned by this method.  If this parameter is -1, then all
        the pending events are returned.
    */
    virtual void removeAll(EventContainer& eventList, int destThrIdx,
                           int maxEvents = -1) override;

    /** The polymorphic destructor.

        The destructor clears up the memory allocated for all the sub
        queues in the constructor.
    */
    virtual ~MultiNonBlockingMTQueue() override;

protected:
    /** The list of sub-queues used to try and minimize contention
        even with lock-free queues.
        

        This list contains the sub-queues that ultimately manage the
        events added to this multi-queue.  Each sub-queue is lock free.

        \note The number of sub-queues is fixed when this class is
        instantiated.  The maximum number of entries in each sub-queue
        is also fixed.

        \note We unfortunately have to use pointers here because some
        of the older boost implementations do not support emplace
        creatable property.
    */
    std::vector<LockFreeQueue*> subQueues;

    /** The bit-mask used for bit-wise operations used to identify
        sub-queue.

        Typically a modulo (%) operation would be used to identify
        sub-queues (e.g., subQidx = srcThrIdx % subQueues.size()).
        However, modulo is an expensive division operation.  So
        bit-wise operations are used, as in -- subQidx = srcThrIdx &
        bitMask.  Accordingly, the bitMask is set to the nearest power
        of two that is less than subQueues.size().  For a even
        distribution of usage, the sub-queue count should be a power
        of 2.
    */
    int bitMask;

private:
    /** Maximum number of events to be removed from a sub-list in the
        removeAll method.

        When removing events, we don't want to spin in removeAll
        method, endlessly consuming events as other threads produce
        them.  We want to get a batch of events and move on.  Since
        LockQueueQueue does not have a size() method to tell number of
        elements in it, we use the maxBatchSize value instead.
    */
    int maxBatchSize;
};

END_NAMESPACE(muse);

#endif
