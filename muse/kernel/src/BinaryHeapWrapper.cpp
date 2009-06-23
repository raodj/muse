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
// Authors: Meseret Gebre       meseret.gebre@gmail.com
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
BinaryHeapWrapper::removeFutureEvents(const Event * future_event){
    bool foundAtleastOne = false;
    //we get to iterate over the heap aka vector !!
    EventContainer * temp = new EventContainer;
    while (!the_container->empty()){
        //std::cout << "Checking: " << the_container->back() << " against: " << future_event<<std::endl;
        if ( the_container->back()->getSenderAgentID() == future_event->getSenderAgentID() &&
             the_container->back()->getReceiveTime() >= future_event->getReceiveTime()   ) {
            //std::cout << "Purging: " << the_container->back() << std::endl;
            the_container->back()->decreaseReference();
            the_container->pop_back();
            foundAtleastOne = true;
        }else{
            //std::cout << "Adding: " << the_container->back() << std::endl;
            temp->push_back(the_container->back());
            the_container->pop_back();
        }
    }

    ASSERT (the_container->empty());
    //now delete the container and have its pointer point to temp pointee
    delete the_container;
    the_container = temp;
    
    //lets make sure we still have valid heap
    make_heap(the_container->begin(),the_container->end(), eventComp() );
    return foundAtleastOne;
}

#endif
