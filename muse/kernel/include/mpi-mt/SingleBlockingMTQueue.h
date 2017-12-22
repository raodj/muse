#ifndef MUSE_SINGLE_BLOCKING_MT_QUEUE_H
#define MUSE_SINGLE_BLOCKING_MT_QUEUE_H

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
#include "mpi-mt/MTQueue.h"

BEGIN_NAMESPACE(muse);

/** A simple blocking implementation for multi-threading safe queues.

    This class implements the features of the abstract MTQueue base
    class using a simple blocking queue implementation.  The list
    typically contains events/messages from other threads or those
    received over MPI.  It enables multiple threds to safely
    add/remove events in a MT-safe manner.  Key implementation aspects
    include:

    <ul>
    
    <li>It uses a shared EventContainer (std::vector) as a backing
    store</li>
    
    <li>It uses a given type of mutex to protected the above
    EventContainer from being simultaneously accessed by multiple
    threads.</li>
    
    </ul>

    \note The MutexType template parameter can either be std::mutex or
    muse::SpinLock, thereby enabling two different strategies to
    accomplish critical sections.
*/
template <typename MutexType = std::mutex>
class SingleBlockingMTQueue : public muse::MTQueue {
public:
    /** The default constructor.

        The default constructor currently reserves space in the
        underlying backing store to minimize inital reallocations.

        \param[in] reserve The initial number of entries to reserve to
        minimize initial reallocations.
    */
    SingleBlockingMTQueue(int reserve = 1024);

    /** A copy constructor for use with creating lists of this class.

        The copy constructor for this class.  Note that it does not
        copy the value of mutex.
    */
    SingleBlockingMTQueue(const SingleBlockingMTQueue& src) :
        queue(src.queue) {
        // Nothing else to be done.
    }
    
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
        added.  Existing entries in this list are left unmodified.
        
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
    virtual ~SingleBlockingMTQueue() override {}

protected:
    /** The backing (not MT-safe) store used by this queue.

        This event container (a std::vector) contains the events added
        (but not yet removed) to this queue.  Access to this list is
        protected using queueMutex;
    */
    EventContainer queue;

    /** A mutex (std::mutex or muse::SpinLock) to enable MT-safe
        access to queue.

        This mutex is locked/unlocked in the different methods in this
        class to enable MT-safe operations on the queue.  The actual
        type is a template parameter.
    */
    MutexType queueMutex;
    
private:
    // Currrently this class does not have any private instance
    // variables.
};

// Since this is a template class the C++ source should be included
// here as part of the header compile unit to ensure consistent
// template parameter usage.
#include "mpi-mt/SingleBlockingMTQueue.cpp"

END_NAMESPACE(muse);

#endif
