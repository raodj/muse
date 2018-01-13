#ifndef NUMA_MEMORY_MANAGER_H
#define NUMA_MEMORY_MANAGER_H

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

#include "config.h"

#if USE_NUMA == 1

#include <numa.h>
#include <numaif.h>
#include <stack>
#include <vector>
#include <unordered_map>
#include "DataTypes.h"
#include <mutex>

BEGIN_NAMESPACE(muse);

// Forward declarations
class MultiThreadedSimulationManager;
class RedistributionMessage;

/** An unordered map to store free events of different sizes.

    The key into this unordered map is the size of the event being
    recycled.  For each size a stack is maintained to return the
    most recently used event to improve cache performance.
*/
using RecycleMemMap = std::unordered_map<int, std::stack<char*>>;

/** An list of free events of different sizes on each NUMA node.

    The index into this list is the numaIDs on the different NUMA
    nodes available on the machine.  Each entry contains a free store
    of events to enable quick recycling of memory.
*/
using NumaRecycleList = std::vector<RecycleMemMap>;

/** \brief Alignment to store NUMA node ID of a memory chunk.

    The NUMA node ID (a zero-based unsigned integer) is stored in an
    aligned location prior to a memory block returned by the
    allocate() method in this memory manager.  This constant
    determined the bytes of alignment (and the extra memory to
    reserve) to store the NUMA node ID.
*/
constexpr int EventAlignment = alignof(muse::Time);

/** A convenience class used by EventRecycler to perform NUMA-aware
    memory allocation.

    This class is used as a thread-local object in EventRecycler.
    This class manages event recycling while being NUMA aware.
    Moreover, this class also operates additional lower level memory
    management -- that is it manages raw NUMA pages.
*/
class NumaMemoryManager {
    friend class EventRecycler;
public:
    /** The default constructor.

        The constructor does not do much operation.  Most of the
        intialization is done by the setup method in this class.
    */
    NumaMemoryManager() : blockSize(65536), allocCalls(0), deallocCalls(0),
                          recycleHits(0), numRedist(0) {}

    /** The destructor.

        The destructor frees up the NUMA pages allocated throughout
        the lifetime of this object.
    */
    ~NumaMemoryManager();

    /** Initial per-thread setup for NUMA memory management.

        This method is called on each thread to initialize the
        per-thread NUMA memory managemnt.  This method allocates 1
        initial page on each of the NUMA nodes in preparation for
        serving events on demand.

        \param[in] numaIDofThread The list of NUMA nodes on which
        different threads have been allocated.

        \param[in] blkSize The size (in bytes) in which NUMA blocks
        are to be allocated.  The default value is 64 KiB.
    */
    void start(const std::vector<int>& numaIDofThread,
               const int blkSize = 65536);

    /** Method to get a block of memory on a given NUMA node.

        This method operates in the following manner (in the order of
        least-to-most operations to perform):
        
        <ol>

        <li>If the NumaEventRecycler for the given NUMA node has an
        unused entry of the given size, this value is immediately
        returned.  This is the shortest/fastest operating path.</li>

        <li>If the current NumaBlock has sufficient memory, then the
         necessary size is allocated from it and returned.</li>

        <li>Otherwise, a new NumaBlock is allocated.  Next the
        necessary size is allocated from it and returned.</li>
        
        </ol>

        \param[in] numaID The NUMA node on which the block of memory
        is to be allocated.

        \param[in] size The size (in bytes) of the memory to be
        allocated.  This would be exactly the same size that would be
        passed to C++'s new operator.
        
        \return A block of memory of given size on the specified NUMA
        node.
    */
    char* allocate(const int numaID, const int size);

    /** Method to deallocate a block of memory on a given NUMA node.

        This method adds the specified block of memory to be reused at
        a later date.

        \param[in] mem The block of memory being deallocated.  This
        pointer should be exactly the same value returned by allocate.
        
        \param[in] size The size (in bytes) of the memory being
        deallocated.  This value should be the exactly the same value
        used to allocate the specified block of memory.
    */
    void deallocate(char *mem, const int size);

    /** Move all pending NUMA deallocations to the main thread's
        memory manager.

        This method is called from MultiThreadedSimulation::simulate()
        method at the end of simulation (just before threads join) to
        move all pending NUMA blocks to the main thread's NUMA memory
        manager.  This is done because the memory cannot be released
        until all agent's have been finalized and their events have
        been reclaimed.  Consequently, pending NUMA blocks are moved
        to the main thread, that frees the memory at the end of
        simulation.
    */
    void moveNumaBlocksTo(NumaMemoryManager& mainMgr);

    /** Method to print internal statistics.

        This method returns a string containing a human-readable form
        of the internal statistics maintained by this NUMA memory
        manager.

        \return A string containing statistics informaton that can be
        readily printed.
    */
    std::string getStats() const;

    /** Method to check and redistribute event memory blocks to
        various threads.

        Depending on communication patterns in the model/simulation,
        memory usage across numa-nodes can be skewed causing
        degradation in overall memory usage and recycling performance.
        This method is periodically called from
        MultiThreadedSimulation::garbageCollect method to check and
        redistribute memory blocks.

        \param[in] threadsPerNode The total number of threads to which
        memory can be redistributed.

        \param[in] threadID The ID of the local thread (0 <= threadID
        < threadsPerNode) associated with this memory manager.

        \param[in] numaID The preferred NUMA node for the given
        thread.
        
        \param[in] mgr The simulation manager to be used to send the
        free memory blocks to other threads.
    */
    void redistribute(const int threadsPerNode, const int threadID,
                      const int numaID,
                      MultiThreadedSimulationManager* const mtc);

    /** Method to add redistributed memory blocks from another thread
        to this thread.

        This method is used to process the messages generated by the
        overloaded redistribute method.  This method is invoked from
        MultiThreadedSimulation's processIncomingEvents method.  The
        message with redistributed memory pointers is passed in as the
        parameter.  This method uses the information in the event to
        add the redistributed memory locations to the appropriate
        entries in the recycler.

        \param[in] rdm The redistribution message to be processed by
        this method.  This parameter cannot be NULL.
    */
    void redistribute(RedistributionMessage* rdm);
    
    /** Convenience method to determine the memory allocated by this
        manager on a given numa node.


        This method adds all the memory allocated on a given NUMA node
        and returns it.

        \param[in] numaID The ID of the numa node whose memory usage
        is to be returned by this method.
        
        \return The memory (in bytes) allocated on this NUMA node.
    */
    int getAllocatedMemory(const int numaID) const;


    /** Convenience method to determine the total amount of recycled
        memory in recycler.

        This method returns all the memory (on a given NUMA node) in the
        recycler.
        
        \param[in] numaID The ID of the numa node whose recycled
        memory availability is to be returned by this method.
        
        \return The memory (in bytes) available in the recycler for
        the given NUMA node.
    */
    int getRecycledMemory(const int numaID) const;
    
protected:
    /** A convenience wrapper to encapsulate information about a
        single contiguous block of NUMA memory.
        
        This class is internally used by the NumaMemoryManager to
        track the numa blocks allocated by the NumaMemoryManager.
    */
    struct NumaBlock {
        /** ID of the NUMA node.  This value is never changed once a
            block has been created */
        int    numaID;
        /** Starting address used to free blocks at end of use.  This
            value is never changed once a block has been created */
        char*  start;
        /** The current starting address of free space in this block.
            This pointer is updated each time a chunk of memory is
            used from this block. */
        char*  current;
        /** Available memory (in bytes) in this block.  This value is
            decreased each time a chunk of memory is used from this
            block. */
        size_t avail;
    };

    /** Allocate and add a new block of NUMA memory for use on a given
        NUMA node.

        This method is used to allocate a block of memory on a given
        NUMA node.  Blocks are allocated based on blockSize.
        
        \param[in] numaID The ID of the node on which the memory is to
        be allocated.
    */
    void allocateBlock(const int numaID);
    
    /** \brief A slightly different version adapted from standard
        implementation of glibc.

        \note This method is slightly different than the standard
        align method.
        
        This function tries to fit __size bytes of storage with
        alignment __align into the buffer __ptr of size __space bytes.
        If such a buffer fits then __ptr is changed to point to the
        __space bytes after the aligned storage and __space is reduced by
        the bytes used for alignment.
        
        \param[in] __align   A fundamental or extended alignment value.
        \param[in] __size    Size of the aligned storage required.
        \param[in] __ptr     Pointer to a buffer of __space bytes.
        \param[in] __space   Size of the buffer pointed to by __ptr.
        \return the updated pointer if the aligned storage fits,
        otherwise nullptr.
    */
    static inline char* alignPtr(size_t __align, size_t __size, char*& __ptr,
                                 size_t& __space) noexcept  {
        const uintptr_t  __intptr = reinterpret_cast<std::uintptr_t>(__ptr);
        const uintptr_t __aligned = (__intptr - 1u + __align) & -__align;
        const size_t __diff       = __aligned - __intptr;
        if ((__size + __diff) > __space) {
            return nullptr;
        } else  {
            __space -= (__diff + __size);
            __ptr    = (reinterpret_cast<char*>(__aligned) + __size);
            return reinterpret_cast<char*>(__aligned);
        }
    }
    
private:
    /** The number of pages to allocate whenever a new block if needed.

        This instance variable tracks the number of pages to allocate
        when a new block is needed.  This value is in number of bytes.
        The default is 64KiB.
    */
    size_t blockSize;
    
    /** The set of deallocated memory ready to be reused.

        This list maintains the set of memory chunks that have been
        deallocated -- that is ready to use chunks of given size.  The
        NUMA-node ID (zero-based index) is used as the index into this
        vector for quick access to memory on a specific NUMA-node.
        Entries are added to this list in the
        NumaMemoryManager::deallocate method.  Entries are removed
        from this list in the NumaMemoryManager::allocate method.
    */
    NumaRecycleList recycler;
    
    /** The list of NUMA blocks allocated by this manager.

        The index into this list is the numaIDs on the different NUMA
        nodes available on the machine.  Each entry contains a stack
        of NumaBlock objects.  The top-of-stack is the currently used
        (possibly with space) block from which requests for memory are
        met.  This list tracks the NUMA blocks allocated by this
        manager.  This list is used to free-up the NUMA memory blocks
        in the destructor.
    */
    std::vector<std::stack<NumaBlock>> blockList;

    /** Mutex used to serialize access to numa_alloc and
        moveNumaBlocksTo method.

        An internal mutex used by the NUMA memory manager to serialize
        operations to some of the system calls and internal operations
        that require thread-safety as they use shared resources.
    */
    static std::mutex numaMutex;

    /** The number of method calls to the allocate method in this
        class.

        This counter tracks the number of calls to the allocate
        method.
    */
    size_t allocCalls;

    /** The number of deallocation method calls.

        This counter tracks the number of deallocate calls.

        \note With events going across threads, the number of allocate
        vs. deallocate calls will not match-up.  However, the total
        number across multiple threads should all add-up.
    */
    size_t deallocCalls;

    /** The number of times memory requests were satisfied from event
        recycler.

        This counter tracks the number of times an allocation request
        was satisfied using a recycled memory.  Having a high hit rate
        is important for efficient NUMA utilization.
    */
    size_t recycleHits;

    /** The number of times this memory manager redistributed events.

        This counter tracks the number of times redistribution of
        events was performed by this memory manager.  This value is
        reported in the statistics.
     */
    int numRedist;
};

END_NAMESPACE(muse);

#endif  // USE_NUMA

#endif
