#ifndef MUSE_MT_QUEUE_H
#define MUSE_MT_QUEUE_H

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

#include "DataTypes.h"

BEGIN_NAMESPACE(muse);

/** An abstract base class for multi-threading safe queues.

    This is an abstract base class used to streamline interaction with
    different types of multi-threading safe queues.  The primary
    objective of the MTQueue is to permit multiple threads to perform
    add/remove operations.  The derived classes perform the actual
    tasks using different strategies.  MTQueue is typically used to
    provide unordered list of events as input to
    MultiThreadedSimulation.  The list typically contains
    events/messages from other threads or those received over MPI.

    \note The various methods (implemented by derived classes) do not
    modify/update the reference counters on the events.  The events
    remain untouched by this hierarchy of classes.  Consequently, it
    is the callers responsibility to appropriately manage the
    reference counts on the events via Event:increaseReference and
    Event::decreaseReference API methods.
*/
class MTQueue {
public:
    /** Add an event into the queue.

        This method adds a given into the input queue.  This method
        must be implemented in a MT-safe (multi-threading safe) manner
        so that multiple threads can simultaneously call this method
        without experiencing race conditions.

        \param[in] srcThrIdx A logical zero-based index of the source
        thread that is sending the event.  This value could be -1 if
        the source thread is on a different process.

        \param[in] destThrIdx The index of the destination thread on
        the local process which will process the event.
        
        \param[in] event Pointer to a MUSE event.  This pointer cannot
        be NULL.
    */
    virtual void add(int srcThrIdx, int destThrIdx,
                     muse::Event* event) = 0;

    /** Add a big batch of events into the queue.

        This method adds events from a given list into the input
        queue.  This method must be implemented in a MT-safe
        (multi-threading safe) manner so that multiple threads can
        simultaneously call this method without experiencing race
        conditions.

        \param[in] srcThrIdx A logical zero-based index of the source
        thread that is sending the event.  This value could be -1 if
        the source thread is on a different process.

        \param[in] destThrIdx The index of the destination thread on
        the local process which will process the event.
        
        \param[in] eventList The list of events to be added to this
        queue.
    */
    virtual void add(int srcThrIdx, int destThrIdx,
                     EventContainer& eventList) = 0;

    /** Remove all the events in this queue into a local list.

        This method performs bulk copying of all the events (if any)
        added to this queue into a separate list and returns the list
        of events.

        \note The list returned by this method is not MT-safe
        
        \return This method returns a list (std::vector) of events
        added to this queue.  If the queue was empty, then the list is
        also empty.  The list returned by this method is not MT-safe.

        \param[in] numEvents The maximum number of events to be
        returned by this method.  If this parameter is -1, then all
        the pending events are returned.
        
        \param[in] destThrIdx The index of the destination thread on
        the local process which will process the event.

        \return This method returns a list (std::vector) of events
        added to this queue.  If the queue was empty, then the list is
        also empty.  The list returned by this method is not MT-safe.        
    */
    virtual EventContainer removeAll(int destThrIdx,
                                     int maxEvents = -1) = 0;
    
    /** The polymorphic destructor.

        The destructor does not perform any specific operation and is
        merely present to enable polymorphic deletion of MTQueue.
    */
    virtual ~MTQueue() {}

protected:
    /** The default constructor.

        <p>This class is an abstract base class and should not be
        directly instantiated.  Consequently the constructor is
        protected.  Instead instantiated and use one of the derived
        classes.</p>

        <p>The default constructor does not perform any
        operations.</p>
    */
    MTQueue() {}
};

END_NAMESPACE(muse);

#endif
