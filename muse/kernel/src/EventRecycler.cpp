#ifndef MUSE_EVENT_RECYCLER_CPP
#define MUSE_EVENT_RECYCLER_CPP

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

#include <mutex>
#include "EventRecycler.h"

// Just to keep namespaces manageable
using namespace muse;

// The static map used for recycling events
thread_local RecycleMap EventRecycler::Recycler;

// The list of pending output events to be deallocated/recycled
thread_local EventContainer EventRecycler::pendingDeallocs;

int
EventRecycler::getReferenceCount(const muse::Event* const event) {
    return event->referenceCount;
}

int
EventRecycler::getInputRefCount(const muse::Event* const event) {
    return event->inputRefCount;
}

void
EventRecycler::decreaseReference(muse::Event* const event) {
    ASSERT(event != NULL);
    ASSERT(event->getReferenceCount() > 0);
    // Decrement the reference count.
    event->referenceCount--;
    ASSERT(event->getReferenceCount() >= 0); 
    if (event->referenceCount == 0) {
        // Manually call event destructor
        event->~Event();
        // Free-up or recycle the memory for this event.
#ifdef RECYCLE_EVENTS        
        deallocate(reinterpret_cast<char*>(event), event->getEventSize());
#else
        // Avoid 1 extra call to getEventSize() in this situation
        deallocate(reinterpret_cast<char*>(event), 0);
#endif
    }
}

// ------------[ Methods associated with event recycling ]--------------

char*
EventRecycler::allocate(const int size) {
    // Don't call processPendingDeallocs here.  It turned out to be
    // too expensive a large number of events can be pending and
    // iterating over the list was taking ~65% of the runtime instead
    // of the desirable ~5% it should.
    // processPendingDeallocs();  // <-- A *big* no, no!
#ifdef RECYCLE_EVENTS
    RecycleMap::iterator curr = Recycler.find(size);
    if (curr != Recycler.end() && !curr->second.empty()) {
        // Recycle existing buffer.
        char *buf = curr->second.top();
        curr->second.pop();
        return buf;
    }
#endif
    // No existing buffer of given size to recycle (or recycling is
    // disabled). So, create a new one.
    return new char[size];
}

void
EventRecycler::deallocate(char* buffer, const int size) {
#ifdef RECYCLE_EVENTS
    Recycler[size].push(buffer);
#else
    UNUSED_PARAM(size);
    delete [] buffer;
#endif
}

void
EventRecycler::deleteRecycledEvents() {
    // Clear out any pending deallocations first.  This method is
    // called just once or twice at the end of simulation to
    // clean-up. So performance is not an issue but aggressive
    // clean-up is important.
    processPendingDeallocs();
    // Now finally delete recycled events.
    for (RecycleMap::iterator curr = Recycler.begin();
         (curr != Recycler.end()); curr++) {
        std::stack<char*>& stack = curr->second;
        while (!stack.empty()) {
            delete [] stack.top();
            stack.pop();
        }
    }
    Recycler.clear();
}

// This method is designed to be used only in multi-threaded mode when
// directly sharing events between threads.  This method should be
// called only from the thread that sent the event!
void
EventRecycler::decreaseOutputRefCount(muse::Event* const event) {
    ASSERT(event != NULL);
    ASSERT(event->getReferenceCount() > 0);
    // Decrement the reference count.
    event->referenceCount--;
    ASSERT(event->getReferenceCount() >= 0); 
    if (event->referenceCount == 0)  {
        if (event->inputRefCount == 0) {
            // Both threads are done with this event!
            // Manually call event destructor
            event->~Event();
            // Free-up or recycle the memory for this event.
#ifdef RECYCLE_EVENTS        
            deallocate(reinterpret_cast<char*>(event), event->getEventSize());
#else
            // Avoid 1 extra call to getEventSize() in this situation
            deallocate(reinterpret_cast<char*>(event), 0);
#endif
        } else {
            // This event is still being used by the receiving
            // thread. Queue it to be reclaimed later.
            ASSERT(event->referenceCount == 0);
            pendingDeallocs.push_back(event);
        }
    }  // event->refCount == 0
}

double
EventRecycler::processPendingDeallocs() {
    if (pendingDeallocs.empty()) {
        return -1;  // No events pending to be deallocated
    }
    // These 2 are counters to track fraction of deallocated events
    int fullSize  = pendingDeallocs.size(), delCount = 0;
    // The next two are counters to iterate over the list.
    int lastEntry = pendingDeallocs.size() - 1, idx = lastEntry;
    while (idx >= 0) {
        muse::Event* const event = pendingDeallocs[idx];
        ASSERT(event != NULL);
        ASSERT(event->referenceCount == 0);
        if (event->inputRefCount == 0) {
            // Remove event for recycling.  To make removal faster, we
            // swap with last entry and pop it off
            std::swap(pendingDeallocs[idx], pendingDeallocs[lastEntry]);
            pendingDeallocs.pop_back();  // Remove dealocated event
            lastEntry--;                 // Account for removed event.
            // Deallocate the current event.
            deallocate(reinterpret_cast<char*>(event), event->getEventSize());
            // Track number of events deallocated
            delCount++;
        }
        // Onto the next entry
        idx--;
    }
    // Return fraction of events deallocated
    DEBUG(std::cout << "Deallocs cleared: " << delCount << " of "
                    << fullSize << std::endl);
    return (delCount / (double) fullSize);
}

void
EventRecycler::movePendingDeallocsTo(EventContainer& mainList) {
    // Ensure the lists are not the same by checking their addresses
    ASSERT(&pendingDeallocs != &mainList);
    // Copy the pending events from this list to the end of the main
    // list in a thread safe manner. This method is not frequently
    // called and consequently having a static mutex here is not a
    // performance issue.
    static std::mutex mainListLock;
    // Lock the mutex so only one thread modifies the mainList.
    std::lock_guard<std::mutex> guard(mainListLock);
    mainList.insert(mainList.end(), pendingDeallocs.begin(),
                    pendingDeallocs.end());
    // Clear out the pending deallocations list as all the entries are
    // now in the mainList.
    pendingDeallocs.clear();
}

#endif
