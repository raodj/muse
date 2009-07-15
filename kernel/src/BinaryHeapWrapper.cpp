#ifndef BINARYHEAPWRAPPER_CPP
#define	BINARYHEAPWRAPPER_CPP

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


#include "BinaryHeapWrapper.h"
#include <algorithm>

using namespace muse;
using std::make_heap;
using std::pop_heap;
using std::push_heap;
using std::sort_heap;

BinaryHeapWrapper::BinaryHeapWrapper() {
    the_container = new EventContainer;
    make_heap(the_container->begin(),the_container->end(), eventComp() );
}

BinaryHeapWrapper::~BinaryHeapWrapper(){ delete the_container; }

void
BinaryHeapWrapper::pop() {
    if (the_container->empty()) return;
    pop_heap(the_container->begin(),the_container->end(),eventComp());
    the_container->back()->decreaseReference();
    the_container->pop_back();
}

void
BinaryHeapWrapper::push(Event * event){
    event->increaseReference();
    the_container->push_back(event);
    push_heap(the_container->begin(),the_container->end(),eventComp());
}

bool
BinaryHeapWrapper::removeFutureEvents(const Event* antiMsg){
    bool foundAtLeastOne = false;
    EventContainer* temp = new EventContainer;
    
    while (!the_container->empty()) {
        // First, get a pointer to the last event, and remove it from
        // the container (it will be either deleted or requeued as
        // necessary)
        Event* lastEvent = the_container->back();
        the_container->pop_back();
        // An event is deleted only if the *sent* time is greater than
        // the antiMessage's and if the event is from same sender
        if ((lastEvent->getSenderAgentID() == antiMsg->getSenderAgentID()) &&
            (lastEvent->getSentTime() >= antiMsg->getSentTime())) {
            // This event needs to be cancelled
            lastEvent->decreaseReference();
            foundAtLeastOne = true;
        } else {
            // This event is still valid, and needs to be processed
            temp->push_back(lastEvent);
        }
    }

    ASSERT (the_container->empty());

    // Replace the container with the new one we just created
    delete the_container;
    the_container = temp;
    
    // Fix up the heap
    make_heap(the_container->begin(),the_container->end(), eventComp());
    
    return foundAtLeastOne;
}

#endif
