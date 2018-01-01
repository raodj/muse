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
#include <sstream>
#include "EventRecycler.h"
#include "mpi-mt/MultiThreadedCommunicator.h"

// Just to keep namespaces manageable
using namespace muse;

// The static information shared between multiple threads and used
// only for NUMA-aware memory management.

// By default NUMA-awareness is disabled.
EventRecycler::NumaSetting EventRecycler::numaSetting =
    EventRecycler::NUMA_NONE;

// The list of NUMA-nodes for each thread.
std::vector<int> EventRecycler::numaIDofThread;

// The shared communicator to provide agent-to-thread mapping.
const MultiThreadedCommunicator* EventRecycler::mtc = NULL;

//-------------[ The following are thread-local statics ]----------------

// The static map used for recycling events
thread_local RecycleMap EventRecycler::Recycler;

// The list of pending output events to be deallocated/recycled
thread_local EventContainer EventRecycler::pendingDeallocs;

// The thread-local index (zero-based) of the thread.
thread_local char EventRecycler::threadID = 0;

#if USE_NUMA == 1
// The NUMA-aware memory manager for each thread
thread_local NumaMemoryManager EventRecycler::numaMemMgr;
#endif

// The number of method calls to the allocateDefault method in
thread_local size_t EventRecycler::allocCalls = 0;

// The number of deallocationDefault method calls.
thread_local size_t EventRecycler::deallocCalls = 0;

// The number of times memory requests were satisfied from the default
// event recycler
thread_local size_t EventRecycler::recycleHits = 0;

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
        deallocate(event);
    }
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
            deallocate(event);
        } else {
            // This event is still being used by the receiving
            // thread. Queue it to be reclaimed later.
            ASSERT(event->referenceCount == 0);
            pendingDeallocs.push_back(event);
        }
    }  // event->refCount == 0
}

// -------[ Methods for managing shared events between threads ]---------

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
            deallocate(event);
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

// -------[ Methods associated with event recycling non-NUMA ]---------

char*
EventRecycler::allocateDefault(const int size, const muse::AgentID receiver) {
    UNUSED_PARAM(receiver);
    // Don't call processPendingDeallocs here.  It turned out to be
    // too expensive a large number of events can be pending and
    // iterating over the list was taking ~65% of the runtime instead
    // of the desirable ~5% it should.
    // processPendingDeallocs();  // <-- A *big* no, no!

    // Track the number of times this method was called
    allocCalls++;
    
#ifdef RECYCLE_EVENTS
    RecycleMap::iterator curr = Recycler.find(size);
    if (curr != Recycler.end() && !curr->second.empty()) {
        // Recycle existing buffer.
        char *buf = curr->second.top();
        curr->second.pop();
        // Track success here.
        recycleHits++;
        return buf;
    }
#endif
    // No existing buffer of given size to recycle (or recycling is
    // disabled). So, create a new one.
    char *buffer = new char[size];
    // Setup the thread ID for the event being allocated for
    // NUMA-aware recycling.  The logic below assumes that
    buffer[sizeof(muse::Event) - 1] = threadID;
    return buffer;
}

void
EventRecycler::deallocateDefault(char* buffer, const int size) {
    deallocCalls++;
#ifdef RECYCLE_EVENTS
    Recycler[size].push(buffer);
#else
    UNUSED_PARAM(size);
    delete [] buffer;
#endif
}

void
EventRecycler::deleteRecycledEventsDefault() {
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

// -------[ Methods associated with event recycling with NUMA ]---------

void
EventRecycler::setupNUMA(const MultiThreadedCommunicator* comm,
                         const std::vector<int>& numaIDList,
                         EventRecycler::NumaSetting numa) {
#if USE_NUMA == 1    
    // Setup overall/default NUMA settings.
    numaSetting = numa;
    // Save information in static variables for use by all threads.
    mtc = comm;
    numaIDofThread = numaIDList;
    ASSERT(!numaIDofThread.empty());
    ASSERT(mtc != NULL);

#ifndef RECYCLE_EVENTS
        std::cerr << "NUMA use requires that RECYCLE_EVENTS macro be defined "
                  << "during compile time. You will need to recompile MUSE "
                  << "with RECYCLE_EVENTS macro to use NUMA awareness.\n";
        abort();
#endif

#endif // USE_NUMA == 1
}

void
EventRecycler::startNUMA(const int blockSize) {
#if USE_NUMA == 1
    if (numaSetting != NUMA_NONE) {
        // Start the per-thread NUMA operations.
        numaMemMgr.start(numaIDofThread, blockSize);
    }
#endif // USE_NUMA == 1
}

#if USE_NUMA == 1

char*
EventRecycler::allocateNuma(const NumaSetting numaMode, const int size,
                            const muse::AgentID receiver) {
    if (numaMode == NUMA_NONE) {
        // NUMA use is disabled at runtime.
        ASSERT(numaSetting == NUMA_NONE);  // Global setting should be same
        return allocateDefault(size, receiver);
    }
    // Use NUMA-aware memory allocator.
    ASSERT(mtc != NULL);
    const int thrID  = ((numaMode == NUMA_SENDER) ? threadID :
                        mtc->getThreadID(receiver, threadID));
    ASSERT((thrID >= 0) && (thrID < (int) numaIDofThread.size()));
    const int numaID = numaIDofThread[thrID];
    // Let NUMA memory manager give us the desried block of memory
    return numaMemMgr.allocate(numaID, size);
}

void
EventRecycler::deallocateNuma(char* buffer, const int size) {
    if (numaSetting == NUMA_NONE) {
        deallocateDefault(buffer, size);
    }
    // Return memory to NUMA-aware memory manager
    numaMemMgr.deallocate(buffer, size);
}

void
EventRecycler::deleteRecycledEventsNuma() {
    deleteRecycledEventsDefault();
}

#endif  // USE_NUMA

std::string
EventRecycler::getStats() {
    std::ostringstream os;
    os << "  Default Allocate   calls: " << allocCalls
       << "\n  Default Deallocate calls: " << deallocCalls
       << "\n  Default Recycler   hits : " << recycleHits
       << "\n  Default Recycler  %hits : "
       << ((float) recycleHits / allocCalls)
       << std::endl;
    // Return stats information back to the caller
    return os.str();
}

#endif
