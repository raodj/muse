#ifndef BINARY_HEAP_WRAPPER_H
#define BINARY_HEAP_WRAPPER_H

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
// Authors: Meseret Gebre          meseret.gebre@gmail.com
//          Dhananjai M. Rao       raodm@muohio.edu
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include <vector>
#include "DataTypes.h"
#include "Event.h"

using std::vector;

BEGIN_NAMESPACE(muse);

/** \brief Binary (min) Heap based on std::vector

    This class provides a convenient interface to a vector-backed
    heap.  The heap is created and managed using standard methods.

    Currently, the heap is MUSE-specific: it contains Event* only.
    
 */
class BinaryHeapWrapper {
public:
    /** \brief Default constructor.

        The constructor initialzies the vector and creates the
        initial, empty heap.
     */
    BinaryHeapWrapper();

    /** \brief Destructor.

        The destructor deletes all allocated memory, in this case, the
        vector.
     */
    ~BinaryHeapWrapper();

    /** \brief Get the top element of the heap

        This method returns a pointer to the current top elemenet of
        the heap.

        \return A pointer to the top Event
    */
    inline Event* top() const { return heapContainer->front(); }

    /** \brief Remove the top element from the heap

        This method will remove the current top element from the heap,
        and then fixup the heap.

        \note This method does not return the top element from the
        heap -- however, calling this method may cause the Event to be
        deleted as it will decreaseReference().
    */
    void pop();

    /** \brief Push an element onto the heap

        This method will add the specified element to the heap.

        \note This class will call increaseReference() on the event,
        preventing it from being automatically deleted.

        \param[in] event The event to be push into the heap
    */
    void push(Event* event);

    /** \brief Remove all events that occur after the specified Event

        This method will compare the timestamps of all events in the
        heap with that of the specified event. Any Event (from the
        same sender) that occurs at a time greater than or equal to
        that of the specified event will be deleted.
        
        \param[in] antiMsg The Anti-Message indicating what should be
        removed
        
        \return True if any cancellation was done
    */
    bool removeFutureEvents(const Event* antiMsg);

    /** \brief Get the current size of the heap

        \return The current size of the heap        
     */
    inline EventContainer::size_type size() const {
        return heapContainer->size();
    }

    /** \brief Check if the heap is empty

        \return True if the heap is empty
     */
    inline bool empty() const { return heapContainer->empty(); }

private:
    /// The vector backing the heap
    EventContainer* heapContainer;

    /** \brief Event Comparator for min-heap creation

        This class provides a functor for compariing two Event
        instances. It is used internally for heap operations.

        Events are ordered by their recieve times.
        
    */
    class EventComp {
    public:
        inline EventComp() {}
        inline bool operator()(const muse::Event* const lhs, const muse::Event* const rhs) const {
            const Time lhs_time = lhs->getReceiveTime();
            return (lhs_time > rhs->getReceiveTime());
        }
    };
};

END_NAMESPACE(muse);

#endif
