#ifndef MUSE_MULTI_BLOCKING_MT_QUEUE_H
#define MUSE_MULTI_BLOCKING_MT_QUEUE_H

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
#include "mpi-mt/SingleBlockingMTQueue.h"

BEGIN_NAMESPACE(muse);

/** A shared queue with multiple subqueues for exchanging events
    between threads.

    This class implements the features of the abstract MTQueue base
    class using one-or-more separate blocking sub-queues.  Each
    sub-queue list typically contains events/messages from other
    threads or those received over MPI.  It enables multiple threds to
    safely add/remove events in a MT-safe manner.  Key implementation
    aspects include:

    <ul>
    
    <li>It uses a list of SingleBlockingMTQueue for implementing each
    sub-queue</li>
    
    <li>The specified type of mutex is simply passed onto
    SingleBlockingMTQueue for its use.</li>

    <li>The number of subqueues to be used.  Ideally this value must
        be a power of 2 as internally this data structure uses
        bit-wise operations to select a sub-queue based on source
        thread's ID.  </li>
    
    </ul>

    \note The MutexType template parameter can either be std::mutex or
    muse::SpinLock, thereby enabling two different strategies to
    accomplish critical sections.    
*/
template <typename MutexType = std::mutex>
class MultiBlockingMTQueue : public muse::MTQueue {
public:
    /** The default constructor.

        The default constructor currently reserves space in the
        underlying backing store to minimize inital reallocations.

        \param[in] numSubQueues The number of subqueues to be used.
        Ideally this value must be a power of 2 as internally this
        data structure uses bit-wise operations to select a sub-queue
        based on source thread's ID.
        
        \param[in] reserve The initial number of entries to reserve to
        minimize initial reallocations.
    */
    MultiBlockingMTQueue(int numSubQueues = 2, int reserve = 1024);
    
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

        The destructor does not perform any specific operation and is
        merely present to enable polymorphic deletion of MTQueue.
    */
    virtual ~MultiBlockingMTQueue() override {}

protected:
    /** The list of sub-queues used to try and minimize contention for
        a mutex.

        This list contains the sub-queues that ultimately manage the
        events added to this multi-queue.  Each sub-queue has its own
        independent mutex for for MT-safe access.

        \note The number of sub-queues is fixed when this class is
        instantiated.
    */
    std::vector<SingleBlockingMTQueue<MutexType>> subQueues;

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
    // Currrently this class does not have any private instance
    // variables.
};

// Since this is a template class the C++ source should be included
// here as part of the header compile unit to ensure consistent
// template parameter usage.
#include "mpi-mt/MultiBlockingMTQueue.cpp"

END_NAMESPACE(muse);

#endif
