#ifndef NUMA_MEMORY_MANAGER_CPP
#define NUMA_MEMORY_MANAGER_CPP

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

#include "NumaMemoryManager.h"

#if USE_NUMA == 1

#include <numa.h>
#include <numaif.h>
#include <algorithm>
#include <memory>
#include <iostream>
#include <thread>
#include <sstream>
#include "Event.h"

// Switch to muse namespace to streamline code
using namespace muse;

// The shared NUMA mutex
std::mutex NumaMemoryManager:: numaMutex;

std::string
NumaMemoryManager::getStats() const {
    std::ostringstream os;
    // Print top-level aggregate information.
    os << "  NUMA Allocate   calls   : " << allocCalls
       << "\n  NUMA Deallocate calls   : " << deallocCalls
       << "\n  NUMA Recycler   hits    : " << recycleHits
       << "\n  NUMA Recycler  %hits    : "
       << ((float) recycleHits / allocCalls)
       << std::endl;
    // Print block information for each NUMA node.
    os << "  NUMA Blocks: ";
    for (size_t numaID = 0; (numaID < blockList.size()); numaID++) {
        os << numaID << "[" << blockList[numaID].size() << " blks] ";
    }
    os << std::endl;
    // Print recycler information for each NUMA node.
    os << "  NUMA Recycler:\n";
    for (size_t numaID = 0; (numaID < recycler.size()); numaID++) {
        os << "    NUMA node " << numaID << ":";
        const RecycleMemMap& nodeRecycler  = recycler[numaID];
        for (auto curr : nodeRecycler) {
            os << " " << curr.first << " bytes [" << curr.second.size() << "]";
        }
        os << "\n";
    }
    // Return the string containing statistics
    return os.str();
}

// Destructor to free-up all NUMA pages.
NumaMemoryManager::~NumaMemoryManager() {
    // Print final summary information.
    DEBUG(std::cout << getStats() << std::endl);
    // Clear out all unused events.
    recycler.clear();
    // Free up all the pages on all NUMA nodes.
    for (std::stack<NumaBlock>& blockStack : blockList) {
        while (!blockStack.empty()) {
            // Free the numa memory for the top-block.
            NumaBlock& top = blockStack.top();
            numa_free(top.start, blockSize);
            // Remove block from stack
            blockStack.pop();
        }
    }
    // Clear out all the NUMA the pages
    blockList.clear();
}
    
// Method to initialize/setup the NUMA memory manager called on each
// thread.
void
NumaMemoryManager::start(const std::vector<int>& numaIDofThread,
                         const int blkSize) {
    ASSERT(!numaIDofThread.empty());
    ASSERT(blkSize > 1024);
    // Save block size for future use
    blockSize = blkSize;
    // NUMA node numbers start with zero. So find the largest NUMA
    // node up to which memory is to be allocated.
    const int maxNumaID = *std::max_element(numaIDofThread.begin(),
                                            numaIDofThread.end()) + 1;
    // Now setup the initial blocks of memory for the different NUMA
    // nodes.
    blockList.resize(maxNumaID);
    for (int id = 0; (id < maxNumaID); id++) {
        // Use member method to actually allocate 1st block.
        allocateBlock(id);
        // Now we should have at least 1 block
        ASSERT(!blockList[id].empty());
    }
    // Finally create the necessary entries in the memory recycler
    recycler.resize(maxNumaID);
}

void
NumaMemoryManager::allocateBlock(const int numaID) {
    ASSERT((numaID >= 0) && (numaID < (int) blockList.size()));
    char* mem = reinterpret_cast<char*>(numa_alloc_onnode(blockSize, numaID));
    DEBUG(std::cout << "Allocated NUMA block (this= " << this
                    << "): " << static_cast<void*>(mem) << " to "
                    << static_cast<void*>(mem + blockSize) << std::endl);
    DEBUG(std::cout << getStats() << std::endl;);
    ASSERT(mem != NULL);
    // Add a block entry at the top of stack for immediate use.
    blockList[numaID].emplace(NumaBlock{numaID, mem, mem, blockSize});
}

char*
NumaMemoryManager::allocate(const int numaID, const int size) {
    // If setupNuma was called, these asserts will pass correctly.
    ASSERT((numaID >= 0) && (numaID < (int) blockList.size()));
    ASSERT(!blockList[numaID].empty());
    ASSERT(numaID < (int) recycler.size());
    allocCalls++;
    // First check to see if we have an unused block of the given size
    // on the given NUMA node.
    RecycleMemMap& nodeRecycler  = recycler[numaID];
    RecycleMemMap::iterator curr = nodeRecycler.find(size);
    if ((curr != nodeRecycler.end()) && !curr->second.empty()) {
        // Recycle existing buffer.
        char* const buf = curr->second.top();
        curr->second.pop();
        ASSERT(*(buf - EventAlignment) == numaID);
        recycleHits++;
        return buf;
    }
    // When control drops here, we don't have a chunk of memory to
    // recycle. So try to carve out a chunk from the top of the numaPage.
    NumaBlock& top = blockList[numaID].top();
    // Carve out chunk while paying attention to alignment.
    char* mem = alignPtr(EventAlignment, size + EventAlignment,
                         top.current, top.avail);
    ASSERT(top.current <= (top.start + blockSize));
    ASSERT((size_t) (top.current - top.start) == (blockSize - top.avail));
    if (mem == NULL) {
        // The top-block did not have sufficient memory. Allocate new
        // block on the specific NUMA node.
        allocateBlock(numaID); 
        // Carve out chunk while paying attention to alignment.
        NumaBlock& newTop = blockList[numaID].top();
        ASSERT(&newTop != &top);
        mem = alignPtr(EventAlignment, size + EventAlignment,
                       newTop.current, newTop.avail);
    }
    // When control drops here we should have a valid memory location
    // to return.
    ASSERT( mem != NULL );
    // Ok, got memory block. Record numaID at the beginning and return
    // next aligned location back.
    *mem = numaID;
    return (mem + EventAlignment);
}

void
NumaMemoryManager::deallocate(char *mem, const int size) {
    ASSERT(mem != NULL);
    ASSERT(size > 0);
    // Find the numa ID by subtracting event alignment
    char* numaID = (mem - EventAlignment);
    ASSERT((*numaID >= 0) && (*numaID < (int) recycler.size()));
    recycler[*numaID][size].push(mem);
    deallocCalls++;
}

void
NumaMemoryManager::moveNumaBlocksTo(NumaMemoryManager& mainMgr) {
    // Use a mutex to permit only one thread to run through this
    // method.
    std::lock_guard<std::mutex> lock(numaMutex);
    DEBUG(std::cout << getStats() << std::endl);
    // Ensure the lists are not the same by checking their addresses
    ASSERT(&mainMgr != this);    
    // Move all the NumaBlocks to the main thread's memory manager.
    for (size_t numaID = 0; (numaID < blockList.size()); numaID++) {
        std::stack<NumaBlock>& blockStack = blockList[numaID];
        while (!blockStack.empty()) {
            // Add numa block to main list
            mainMgr.blockList[numaID].push(blockStack.top());
            // Remove block from stack
            blockStack.pop();
        }
    }
}

#endif  // USE_NUMA

#endif
