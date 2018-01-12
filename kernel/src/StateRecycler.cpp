#ifndef MUSE_STATE_RECYCLER_CPP
#define MUSE_STATE_RECYCLER_CPP

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

#include "StateRecycler.h"
#include "State.h"

// Switch to muse namespace to streamline source code
using namespace muse;

/** \brief Alignment to store size of state a the beginning of a
    memory chunk.

    The size of state is stored in an aligned location prior to a
    memory block returned by the allocate() method in this memory
    manager.  This constant determines the bytes of alignment (and the
    extra memory to reserve) to store the state size.
*/
constexpr int StateAlignment = alignof(muse::State);

// The per-thread unordered map for recyling states
thread_local StateRecycleMap StateRecycler::Recycler;

// Enable/disable per-thread NUMA-aware memory manager & recycler
thread_local bool StateRecycler::useNuma = false;

// The NUMA-node ID to be used for this thread.
thread_local int StateRecycler::numaID = 0;

#if USE_NUMA == 1
// The thread local NUMA memory manager.
thread_local NumaMemoryManager StateRecycler::numaMemMgr;
#endif

// setup method called on each thread.
void
StateRecycler::setup(bool enableNuma, int numaNodeID) {
    useNuma = enableNuma;   // Copy to thread-local static variables.
    numaID  = numaNodeID;
#if USE_NUMA == 1
    // Create initial blocks on all available numa nodes for now.
    numaMemMgr.start({numaNodeID}, 32768);
#endif
}

char*
StateRecycler::allocate(const int size) {
    ASSERT(size >= (int) sizeof(muse::State));
#ifdef RECYCLE_STATES
    // Recycling enabled.  First check if we have a recycled chunk of
    // memory.  If so return it.
    StateRecycleMap::iterator curr = Recycler.find(size);
    if (curr != Recycler.end() && !curr->second.empty()) {
        // Recycle existing buffer! 
        char *buf = curr->second.top();
        curr->second.pop();
        return buf;
    }
    // No existing buffer to recycle. So, create a new one based on
    // whether NUMA is enabled/disabled.
    const int netSize = size + StateAlignment;
    // NOTE: Calls can come here before multiple threads are used!
#if USE_NUMA == 1
    char *buffer = (useNuma ? numaMemMgr.allocate(numaID, netSize) :
                    static_cast<char*>(::operator new(netSize)));
#else
    char *buffer = static_cast<char*>(::operator new(netSize));
#endif
    // Setup the state size, which is used during deallocation.
    int* sizePtr = reinterpret_cast<int*>(buffer);
    *sizePtr     = size;
    // Return aligned pointer after size (so size is never overwritten)
    return (buffer + StateAlignment);
#else
    // State recycling disabled.
    return static_cast<char*>(::operator new(size));
#endif
}

void
StateRecycler::deallocate(char *state) {
#ifdef RECYCLE_STATES
    // Find size of the state being deallocated from the space before it.
    ASSERT(state != NULL);
    int*  sizePtr  = reinterpret_cast<int*>(state - StateAlignment);
    const int size = *sizePtr;
    ASSERT(size >= (int) sizeof(muse::State));
    // Add entry to appropriate recycler for use later on in allocateDefault
    Recycler[size].push(state);
#else
    // State recycling disabled.
    ::operator delete(state);
#endif
}

void
StateRecycler::deleteRecycledStates() {
#ifdef RECYCLE_STATES
    if (!useNuma) {
        // Now finally delete recycled memory blocks.
        for (StateRecycleMap::iterator curr = Recycler.begin();
             (curr != Recycler.end()); curr++) {
            std::stack<char*>& stack = curr->second;
            while (!stack.empty()) {
                // Get pointer to original allocation location.
                char *mem = stack.top() - StateAlignment;
                // Delete memory
                ::operator delete(mem);
                // Remove entry from recycler.
                stack.pop();
            }
        }
    }
    // Finally clear out all entries in the recycler for good measure.
    Recycler.clear();
#endif
}

#endif
