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
#include "EventAdapter.h"
#include "mpi-mt/RedistributionMessage.h"
#include "mpi-mt/MultiThreadedSimulationManager.h"

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
    os << "  Redistributions: " << numRedist << std::endl;
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
            // Add numa block to first entry in main list.  This may
            // not be the same as the NUMA block but it is not a big
            // deal as we are just going to clean things up.
            mainMgr.blockList.front().push(blockStack.top());
            // Remove block from stack
            blockStack.pop();
        }
    }
}

void
NumaMemoryManager::redistribute(const int threadCount, const int threadID,
                                const int numaID,
                                MultiThreadedSimulationManager* const mgr) {
    ASSERT(threadCount > 0);
    ASSERT((threadID >= 0) && (threadID < threadCount));
    ASSERT((numaID >= 0) && (numaID < (int) blockList.size()));
    ASSERT(mgr != NULL);
    // Find the amount of memory allocated by this manager on the
    // specified numa node to determine if recycling is needed.
    const int memAllocd = getAllocatedMemory(numaID);
    // Find the amount recycled memory we currently have
    const int recyclMem = getRecycledMemory(numaID);
    // If recyclMemory is 2x more than allocated memory, it is time to
    // redistribute.
    if (recyclMem <= memAllocd * 2) {
        // We have not hit the threshold. No further operation needed.
        return;
    }
    DEBUG(std::cout << "Recycled memory: " << recyclMem
                    << " bytes more than 2x the memory allocated: "
                    << memAllocd << std::endl);
    // Compute the proportion of entries to redistribute to each thread.
    const double redistFrac = (recyclMem - memAllocd) / (threadCount - 1.) /
        recyclMem;
    // Redistribute computed fraction of each size. First, get the
    // recycler map for the given numa node.
    RecycleMemMap& nodeRecycler = recycler[numaID];
    // Iterate over each entry (each entry has a different size)
    for (RecycleMemMap::value_type& entry : nodeRecycler) {
        // Compute actual number of memory chunks to send to each thread.
        const int entryCount = std::ceil(redistFrac * entry.second.size());
        // Now ditribute that many entries to each thread.
        for (int thrID = 0; (thrID < threadCount); thrID++) {
            if (thrID == threadID) {
                continue;  // Nothing to do for local thread.
            }
            RedistributionMessage* const rde =
                RedistributionMessage::create(numaID, entryCount, entry.first,
                                              entry.second);
            ASSERT(rde != NULL);
            // Send event to destination thread.
            mgr->addIncomingEvent(thrID, rde);
        }
    }
    // Track the number of redistributions
    numRedist++;
}

void
NumaMemoryManager::redistribute(RedistributionMessage* rdm) {
    ASSERT(rdm != NULL);
    ASSERT(rdm->getNumaID() >= 0);
    ASSERT(rdm->getNumaID() < (int) recycler.size());
    ASSERT(rdm->getEntryCount() > 0);
    ASSERT(rdm->getEntrySize()  > 0);
    // Get the recycler entry corresponding to the NUMA node.
    RecycleMemMap& nodeRecycler = recycler[rdm->getNumaID()];
    // Get the recycler stack for the given size
    std::stack<char*>& stack = nodeRecycler[rdm->getEntrySize()];
    // Now let the redistribution message add entries to this object
    rdm->addEntriesTo(stack);
}

int
NumaMemoryManager::getAllocatedMemory(const int numaID) const {
    ASSERT((numaID >= 0) && (numaID < blockList.size()));
    // Add the memory allocated -- each block is of fixed blockSize
    const int memAllocd = blockList[numaID].size() * blockSize -
        blockList[numaID].top().avail;
    return memAllocd;
}

int
NumaMemoryManager::getRecycledMemory(const int numaID) const {
    ASSERT((numaID >= 0) && (numaID < (int) recycler.size()));
    // Get the recycler map for the given numa node.
    const RecycleMemMap& nodeRecycler  = recycler[numaID];
    // Add-up memory of various sizes.
    int totMem = 0;
    for (const RecycleMemMap::value_type& entry : nodeRecycler) {
        // Compute memory for entry. entry.first is size (in bytes)
        // and entry.second.size() gives number of entries in stack
        totMem += (entry.first * entry.second.size());
    }
    return totMem;
}

#endif  // USE_NUMA

#endif
