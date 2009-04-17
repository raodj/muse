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
// Authors: Meseret Gebre       meseret.gebre@gmail.com
//
//---------------------------------------------------------------------------


/* 
 * File:   BinaryHeapWrapper.h
 * Author: Meseret Gebre - meseret.gebre@gmail.com  - meseretgebre.com
 *
 * Created on April 15, 2009, 12:04 AM
 *
 * This wrapper classes is meant to make a vector container act as a binary heap.
 * Priority Queueu can be used, but we needed access to elements in the heap and
 * priority_queue does not have support for iterators.
 *
 * @note I appened Wrapper because it has special features made for MUSE and we wrap a vector
 */

#ifndef BINARYHEAPWRAPPER_H
#define	BINARYHEAPWRAPPER_H

#include <vector>
#include "DataTypes.h"
#include "Event.h"

using std::vector;

BEGIN_NAMESPACE(muse); //begin namespace muse


class BinaryHeapWrapper {

public:

    /** The ctor
     */
     BinaryHeapWrapper ();

     /** The dtor
     */
     ~BinaryHeapWrapper ();

     /** The top method.
         Nothing special, gives access to the min event in the heap

         @return event pointer, a pointer to the min event.
      */
     inline Event * top() const { return the_container->front();}

     /** The pop method.
        This will return the next heap event.

        @return event, the event with highest priority.
     */
     void pop(void);

     /** The push method.
         This will push the heap element into the heap.

        @param event, the event to be push into the heap.
     */
     void push(Event * event);

     /** The  removeFutureEvents method.
         This will cancel all elements (events) with timestamp equal or greater
         to the passed in element (event) from the sender agent of the event.

         @param future_event, the event to use for cancellation.
         @return bool, if any cancellation was done, true will be returned.
      */
     bool removeFutureEvents(const Event * future_event);

     inline EventContainer::size_type size() const {return the_container->size();}
     inline bool empty() const { return the_container->empty(); }

private:

    /** This is the container that is used with the wrapper.
        When the user wishes to iterate, a pointer to this class
        is returned.
     */
    EventContainer * the_container;

    /** The eventComp class.
        Compares delivery times. puts the one with the smaller ahead.
    */
    class eventComp{
    public:
        eventComp(){}
        inline bool operator() ( muse::Event *lhs,  muse::Event *rhs) const{
            Time lhs_time = lhs->getReceiveTime(); //hack to remove warning during compile time
            return (lhs_time > rhs->getReceiveTime());
        }
    };

};

END_NAMESPACE(muse); //end namespace muse

#endif	/* BINARYHEAPWRAPPER_H */

