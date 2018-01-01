#ifndef MUSE_STATE_RECYCLER_H
#define MUSE_STATE_RECYCLER_H

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

#include <unordered_map>
#include <stack>
#include "NumaMemoryManager.h"

BEGIN_NAMESPACE(muse);

// Forward declaration for some of the classes.
class State;

/** An unordered map to store free states of different sizes.

    The key into this unordered map is the size of the state being
    recycled.  For each size a stack is maintained to return the most
    recently used memory-chunk to improve cache performance.
*/
using StateRecycleMap = std::unordered_map<int, std::stack<char*>>;

/** A convenience class for enabling/disabling (at compile time) state
    recycling.

    This method streamlines the process of recycling states.  State
    recycling is enabled via the RECYCLE_STATES compiler flag.
    Furthermore, memory allocation is NUMA-aware if NUMA libraries are
    available and it has been enabled.  Note that NUMA-awareness is
    used only in multi-threaded simulations.
    
    \note All methods in this class are intentionally designed to be
    static methods as they can be called from multiple threads.

    \note Methods in this class are called well before simulations
    commence to allocate initial states for agents.

    \see muse::State
*/
class StateRecycler {
    friend class MultiThreadedSimulation;
public:
    /** Setup NUMA-aware memory management.

        This method is called only from multi-threaded simulations to
        enable NUMA-aware memory operations.  By default
        NUMA-awareness is disabled.

        \param[in] enableNuma This flag provides runtime option to
        enable or disable NUMA operations (via \c --no-numa
        command-line argument).

        \param[in] numaNodeID The default/preferred NUMA node to be
        used for this thread.

        \see MultiThreadedSimulation::simulate
        \see MultiThreadedSimulationManager::initialize
    */
    static void setup(bool enableNuma, int numaNodeID);

    /** Allocate a block of memory for creating/storing state.

        This method is called from operator new in muse::State.  This
        method recycles/allocates the specified size chunk of memory
        and returns a pointer to it.  If state recycling is disabled
        then this method uses system's new operator to allocate memory.

        If NUMA-awareness is enabled (both at compile and runtime)
        then memory returned by this method is on the preferred NUMA
        node for this thread.

        \param[in] size The size of the memory block to be allocated.
    */
    static char* allocate(const int size);

    /** Recycle/deallocate a block of memory.

        This method is the dual of the allocate method in this class.
        This method is called from the operator delete in muse::State.
        This method recycles/deallocates the memory block.
        
        \param[in] state Pointer to the state to be
        recyled/deallocated.  This pointer must be exactly the same
        pointer returned by a previous call to the allocate method in
        this class.
     */
    static void deallocate(char *state);

    /** Clear out all recycled memory blocks.

        This method is called at the end of simulation to delete all
        the recycled memory blocks (if any).  If state recycling is
        not enabled, then this method does not perform any tasks.
    */
    static void deleteRecycledStates();

#if USE_NUMA == 1
    /** Move all pending NUMA deallocations to the main thread's
        memory manager.

        This method is called from MultiThreadedSimulation::simulate()
        method at the end of simulation (just before threads join) to
        move all pending NUMA blocks to the main thread's NUMA memory
        manager.  This is done because the memory cannot be released
        until all agent's have been finalized and their states have
        been reclaimed.  Consequently, pending NUMA blocks are moved
        to the main thread, that frees the memory at the end of
        simulation.

        \param[out] mainMgr The main memory manager to which all NUMA
        blocks should be moved.
    */
    static void moveNumaBlocksTo(NumaMemoryManager& mainMgr) {
        numaMemMgr.moveNumaBlocksTo(mainMgr);
    }
#endif

protected:
    /** An unordered map of stacks to recycle memory chunks of
        different sizes.

        This map is used only if RECYCLE_STATES macro has been defined
        at compile time.  It has thread local storage so that each
        thread (in case multi-threaded mode is used) get's its own
        recycler thereby eliminating contention/locking.
        
        The key into this unordered map is the size of the memory
        being recycled.  For each size a stack is maintained to return
        the most recently used event to improve cache performance.

        This map is used by the allocate and deallocate methods in
        this class.
    */    
    thread_local static StateRecycleMap Recycler;

    /** Runtime option to enable/disable NUMA usage.

        This flag provides an option to disable NUMA usage in
        multi-threaded simulations.  This flag is intiialized to false
        and is changed in the setup method in this class.
    */
    thread_local static bool useNuma;

    /** The default/preferred NUMA node for this thread.

        This thread-local variable tracks the default/preferred NUMA
        node for this thread.  This value is initialized to zero.  It
        is updated in the setup() method in this class.
     */
    thread_local static int numaID;
    
#if USE_NUMA == 1
    /** A thread-local NUMA memory manager for managing memory in a
        NUMA-aware manner.

        This is a thread local numa memory manager that performs the
        core tasks of managing memory in a NUMA-aware fashion.
    */
    thread_local static NumaMemoryManager numaMemMgr;
#endif
    
private:
    /** The default constructor.

        It is intentionally marked delete to ensure that this class is
        never instantiated.
     */
    StateRecycler() = delete;

    /** The destructor.
        
        It is intentionally marked delete to ensure that this class is
        never instantiated.
     */
    ~StateRecycler() = delete;
};

END_NAMESPACE(muse);

#endif
