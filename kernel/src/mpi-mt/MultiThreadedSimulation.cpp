#ifndef MUSE_MULTI_THREADED_SIMUALTION_CPP
#define MUSE_MULTI_THREADED_SIMUALTION_CPP

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

#include <numa.h>
#include <numaif.h>
#include <string>
#include "mpi-mt/MultiThreadedSimulationManager.h"
#include "mpi-mt/MultiThreadedCommunicator.h"
#include "mpi-mt/SingleBlockingMTQueue.h"
#include "mpi-mt/MultiBlockingMTQueue.h"
#include "mpi-mt/MultiNonBlockingMTQueue.h"
#include "SpinLock.h"
#include "GVTManager.h"
#include "GVTMessage.h"
#include "Scheduler.h"
#include "ArgParser.h"
#include "EventAdapter.h"
#include "EventRecycler.h"
#include "mpi-mt/RedistributionMessage.h"

// Switch to muse namespace to streamline code
using namespace muse;

// The shared barrier to facilitate coordination of threads to help
// clean-up recycled events.
SpinLockThreadBarrier MultiThreadedSimulation::threadBarrier;

// The static/shared list of NUMA-node IDs for each thread.  This list
// is static and is shared by multiple threads.  Entries are added by
// the derived class, in MultiThreadedSimulationManager::createThreads
std::vector<int> MultiThreadedSimulation::numaIDofThread;

MultiThreadedSimulation::MultiThreadedSimulation(MultiThreadedSimulationManager* mgr,
                                                 int thrID, int globalThrID,
                                                 int threadsPerNode,
                                                 bool usingSharedEvents,
                                                 int cpuID) :
    Simulation(usingSharedEvents), threadsPerNode(threadsPerNode),
    threadID(thrID), globalThreadID(globalThrID), mtCommMgr(NULL),
    simMgr(mgr), mainPendingDeallocs(EventRecycler::pendingDeallocs),
    cpuID(cpuID)
#if USE_NUMA == 1
    , mainNumaManager(EventRecycler::numaMemMgr),
    mainStateRecycler(StateRecycler::numaMemMgr)
#endif
{
    ASSERT(mgr != NULL);
    ASSERT(threadsPerNode > 0);
    ASSERT(thrID >= 0);
    // Initialize counters used to dynamically adapt number of calls
    // to processPendingDeallocs() method from garbageCollect() method
    // in this class to keep simulations fast.
    deallocThresh  = 0.1;
    deallocRate    = 1;
    deallocTicker  = deallocRate;
    // Counter to track number of times the incomingEvents queue's
    // removeAll method was called in the processIncomingEvents()
    // method.
    shrQcheckCount = 0;
    // Nothing much to be done for now as base class does all the
    // necessary work.
}

MultiThreadedSimulation::~MultiThreadedSimulation() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

void
MultiThreadedSimulation::setCommManager(MultiThreadedCommunicator* mtc) {
    // Save pointer to derived class instance so that we don't need to
    // do weird type casting.
    mtCommMgr   = mtc;
    // Set pointer in base class as well
    commManager = mtc;
    // Update the MPI information in the base class
    unsigned int totNumThreads;  // dummy. not really used.
    commManager->getProcessInfo(myID, numberOfProcesses, totNumThreads);
}

void
MultiThreadedSimulation:: initialize(int& argc, char* argv[], bool initMPI)
    throw(std::exception) {
    UNUSED_PARAM(initMPI);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);
}

void
MultiThreadedSimulation::finalize(bool stopMPI, bool delCommMgr) {
    // The base class does all the necessary work
    Simulation::finalize(stopMPI, delCommMgr);
    // Delete our incoming queue as we no longer need it.
    if (incomingEvents != NULL) {
        delete incomingEvents;
        incomingEvents = NULL;  // for sanity checks
    }
}

void
MultiThreadedSimulation::processIncomingEvents() {
    // Get all incoming events in an MT-safe manner.
    incomingEvents->removeAll(shrEvents, threadID);
    // Update queue statistics for reporting at the end
    shrQcheckCount++;
    if (!shrEvents.empty()) {
        shrQevtCount += shrEvents.size();
    }
    // Process each event in a manner similar to base class operations
    for (Event* event : shrEvents) {
        if (event->getSenderAgentID() == -1) {
            // This must be a GVT message.
            ASSERT(event->getReceiverAgentID() <= 0);
            ASSERT(event->getReceiveTime() == -1);
            ASSERT(dynamic_cast<GVTMessage*>(event) != NULL);
            GVTMessage *msg = static_cast<GVTMessage*>(event);
            ASSERT(msg != NULL);
            gvtManager->recvGVTMessage(msg);
        } else if (event->getSenderAgentID() == REDISTR_MSG_SENDER) {
#if USE_NUMA == 1
            // This is a redistribution message to redistribute NUMA
            // operations.
            ASSERT(dynamic_cast<RedistributionMessage*>(event) != NULL);
            RedistributionMessage *rdm =
                static_cast<RedistributionMessage*>(event);
            // Add entries to our thread-local recycler.
            EventRecycler::numaMemMgr.redistribute(rdm);
            // Get rid of this message
            RedistributionMessage::destroy(rdm);
#endif            
        } else {
            // This is a regular event.  All incoming events must be
            // inspected by the GVT manager (for tracking GVT) prior
            // to further processing.
            gvtManager->inspectRemoteEvent(event);
            scheduleEvent(event);
            if (!doShareEvents) {
                // Decrease the reference because if it was rejected,
                // the event will be properly deleted. However, if it
                // is in the eventPQ, it will be unharmed (reference
                // count will actually be fixed to correct for lack of
                // entry of this event in a output queue on this
                // process)
                ASSERT(EventRecycler::getReferenceCount(event) < 3);
                EventRecycler::decreaseOutputRefCount(doShareEvents, event);
            } else {
                // Decrease reference count to counter the temporary
                // increase (see: MultiThreadedSimulation::
                // scheduleEvent(Event* e)
                EventRecycler::decreaseInputRefCount(doShareEvents, event);
            }
        }
        // Note: Don't skip this step -- Let the GVT Manager
        // forward any pending control messages, if needed
        gvtManager->checkWaitingCtrlMsg();
    }
    if (shrEvents.empty()) {
        // Note: Don't skip this step -- Let the GVT Manager forward
        // any pending control messages, if needed
        gvtManager->checkWaitingCtrlMsg();
    } else {
        // Clear out all processed events (capacity is unaltered) in
        // preparation for next round of calls ot this emthod.
        shrEvents.clear();
    }
}

void
MultiThreadedSimulation::setCpuAffinity() const {
    DEBUG(std::cout << "Thread #" << threadID << ", cpuID: "
                    << cpuID << std::endl);
#if USE_NUMA == 1
    // Setup additional information in state recycler.
    StateRecycler::setup(EventRecycler::numaSetting != EventRecycler::NUMA_NONE,
                         numaIDofThread.at(threadID));
#endif
    
    if ((cpuID == -1) || (threadID == 0)) {
        return;  // Nothing else to be done.
    }
    cpu_set_t cpuset;   // cpu bit-flags
    CPU_ZERO(&cpuset);  // clear out the cpu bit-flags
    // Set affinity to only 1 cpu corresponding to thread ID
    CPU_SET(cpuID, &cpuset);
    int err = 0;
    if ((err = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t),
                                          &cpuset)) != 0) {
        std::cerr << "Error in setThreadAffinity(): " << err << std::endl;
        abort();
    }
    // Relocate the stack of the thread to the CPU's NUMA node if
    // NUMA-awareness is available to us.
#if USE_NUMA == 1
    pthread_attr_t attr;
    void *stackAddr  = nullptr;
    size_t stackSize = 0;
    if (((err = pthread_getattr_np(pthread_self(), &attr)) != 0) ||
        ((err = pthread_attr_getstack(&attr, &stackAddr, &stackSize)) != 0)) {
        std::cerr << "Error getting stack information for thread: "
                  << err << std::endl;
        abort();
    }
    // bind the stack for this thread on the NUMA node
    const unsigned long nodeMask = 1UL << getNumaNodeOfCpu(cpuID);
    const long bindRc = mbind(stackAddr, stackSize, MPOL_BIND, &nodeMask,
                              sizeof(nodeMask), MPOL_MF_MOVE | MPOL_MF_STRICT);
    if (bindRc != 0) {
        std::cerr << "Error moving stack to local NUMA memory: "
                  << errno << std::endl;
        abort();
    }    
    // Use some stack space to ensure pages are preallocated.
    const size_t size = numa_pagesize() * 10;
    // Allocate temporary stack space using alloca function.  This
    // memory is automatically cleared when this method returns.
    void *stackMem  = alloca(size);
    // Write data to fault pages -- ensures OS-kernel moves stack.
    memset(stackMem, 0, size);
#endif
}


int
MultiThreadedSimulation::getNumaNodeOfCpu(const int cpu) const {
#if USE_NUMA == 1
    return numa_node_of_cpu(cpu);
#else
    return -1;
#endif
}

// NOTE: This method is the main thread method and is called from
// multiple threads!
void
MultiThreadedSimulation::simulate() {
    // Setup the thread ID in our EventRecycler for initializing
    muse::EventRecycler::threadID = this->threadID;    
    // Setup thread affinity basedon cpuID set for this thread.
    setCpuAffinity();
    // Always, start-up the per-thread NUMA memory manager.
    EventRecycler::startNUMA();
    // Important: wait for threads to finish setup before starting
    // event processing on all threads.
    threadBarrier.wait();
    // Initialize all the agents
    initAgents();
    // Start the core simulation loop.
    LGVT         = startTime;
    int gvtTimer = gvtDelayRate;
    // The main simulation loop
    while (gvtManager->getGVT() < endTime) {
        // See if a stat dump has been requested
        if ((threadID == 0) && doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }
        if (--gvtTimer == 0) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        // Process a block of events received via the network
        // (eventually goes to derived manager class when
        // processMpiMsgs() method is called by the base class).
        checkProcessMpiMsgs();
        // Process incoming events and update GVT token
        processIncomingEvents();
        // Process the next event from the list of events managed by
        // the scheduler.
        if (!processNextEvent()) {
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;            
        }
    }
    // Wait for all the threads to finish by waiting on a barrier.
    threadBarrier.wait();
    // Clear out any pending recyclable events on each thread (as each
    // thread has its own thread_local recycler)
    EventRecycler::deleteRecycledEvents();
    ASSERT(EventRecycler::Recycler.empty());
    threadBarrier.wait();  // Important: wait for threads to finish

#if USE_NUMA == 1    
    // Save NUMA statistics to report later on before cleaning up.
    numaStats = EventRecycler::numaMemMgr.getStats();
#endif
    
    // Add any pending events to the main thread's pending event list
    if (threadID != 0) {
        // This is not the main thread.
        EventRecycler::movePendingDeallocsTo(mainPendingDeallocs);
#if USE_NUMA == 1
        // Add NUMA pages to the main thread's pending event list
        EventRecycler::numaMemMgr.moveNumaBlocksTo(mainNumaManager);
        // Add NUMA pages for state to the main thread
        StateRecycler::moveNumaBlocksTo(mainStateRecycler);
#endif
    }

    // Finally all threads (particularly thread 0) waits for other
    // threads to finish moving pending events (for final clean-up)
    // before exiting this main thread-method.
    threadBarrier.wait();
}

muse::Event*
MultiThreadedSimulation::cloneEvent(const muse::Event* src,
                                    const muse::AgentID receiver) const {
    ASSERT(src != NULL);
    ASSERT(receiver >= 0);
    // First suitably allocate memory for the copy
    const int evtSize = EventAdapter::getEventSize(src);    
    char* mem;  // Initialized in if-else below.
    if (EventRecycler::numaSetting == EventRecycler::NUMA_SENDER) {
        // Override default memory management and allocate event on
        // the receiver's NUMA node.
        mem = EventRecycler::allocateNuma(EventRecycler::NUMA_RECEIVER,
                                          evtSize, receiver);
    } else {
        // Use default memory management of Event.
        mem = Event::allocate(evtSize, receiver);
    }
    ASSERT( mem != NULL );
    muse::Event* const copy = reinterpret_cast<muse::Event*>(mem);
    std::memcpy(copy, src, evtSize);
    ASSERT(copy->getReferenceCount() > 0);
    return copy;
}

bool 
MultiThreadedSimulation::scheduleEvent(Event* e) {
    ASSERT(e->getReceiveTime() >= getGVT());
    ASSERT((doShareEvents == true) || (e->getReferenceCount() == 1) ||
           (e->getReferenceCount() == 2));
    ASSERT(!doShareEvents || (EventRecycler::getInputRefCount(e) > 0) ||
           (EventRecycler::getReferenceCount(e) > 0));
    
    if (TIME_EQUALS(e->getSentTime(), TIME_INFINITY) ||
        (e->getSenderAgentID() == -1)) {
        std::cerr << "Don't use this method with a new event, go "
                  << "through the agent's scheduleEvent method." << std::endl;
        abort();
    }
    // Convenient reference to the destination agent ID
    const AgentID recvAgentID = e->getReceiverAgentID();
    // Find destination thread ID
    const int destThrID = mtCommMgr->getOwnerThreadRank(recvAgentID);
    ASSERT(destThrID != -1);
    if (destThrID == globalThreadID) {
        // Local events are directly inserted into our own scheduler
        return scheduler->scheduleEvent(e);
    } else {
        // Remote events are sent via the GVTManager to aid tracking
        // GVT. The gvt manager calls communicator.
        if (destThrID / threadsPerNode == (int) myID) {
            if (doShareEvents) {
                // We need to increase input reference count (which
                // typically only the receiving thread manipulates,
                // but is safe here as only 1 thread is currently
                // operating with the event) here to ensure that event
                // does not get garbage collected before the receiving
                // thread has a chance to process this event!
                EventRecycler::increaseInputRefCount(e);
                gvtManager->sendRemoteEvent(e);
            } else {
                // Destination thread is on same process but differen
                // thread.  Since this event is crossing thread
                // boundaries we make a copy (while honoring NUMA) to
                // avoid race conditions on the reference counter.
                Event* const copy = cloneEvent(e, recvAgentID);
                gvtManager->sendRemoteEvent(copy);
            }
        } else {
            // The destination thread is on a remote process and event
            // does not need to be cloned.  However, it does need
            // inspection by GVT manager.
            gvtManager->sendRemoteEvent(e);
        }
    }
    return true;
}

void
MultiThreadedSimulation::garbageCollect() {
    // First let base class do its standard garbage collection
    Simulation::garbageCollect();
#if USE_NUMA == 1
    // With NUMA we periodically need to redistribute events to avoid
    // unconstrained memory growth (depending on communication patterns)
    MultiThreadedSimulationManager* const mgr =
        static_cast<MultiThreadedSimulationManager*>(simMgr);
    EventRecycler::numaMemMgr.redistribute(threadsPerNode, threadID,
                                           numaIDofThread[threadID], mgr);
#endif
    // Rest of the logic is needed only when using shared events
    if (!doShareEvents) {
        return;  // Not using shared events. Nothing further to do.
    }
    // Now reclaim/recycle any pending events in an adaptive manner so
    // that we make the most effective use of calls to this method.
    if (--deallocTicker <= 0) {
        // Time to call processPendingDeallocs()
        const double fracReclaimed = EventRecycler::processPendingDeallocs();
        deallocsPerCall    += fracReclaimed;  // Track to report stats at end
        if (fracReclaimed < deallocThresh) {
            // Call processPendingDeallocs() less frequently
            deallocRate *= 2;
            DEBUG(std::cout << "DeallocsRate increased to: " << deallocRate
                            << " using " << fracReclaimed    << std::endl);
        } else if (fracReclaimed > (deallocThresh * 1.5)) {
            // Call processPendingDeallocs() more frequently            
            deallocRate = std::max(1, deallocRate / 2);
            DEBUG(std::cout << "DeallocsRate decreased to: " << deallocRate
                            << " using " << fracReclaimed    << std::endl);
        }
        // Reset the deallocTicker
        deallocTicker = deallocRate;
    }
}

void
MultiThreadedSimulation::parseCommandLineArgs(int &argc, char* argv[]) {
    // Make the arg_record
    std::string mtQueue = "single-blocking";
    int subQueues       = 2;  // #sub-queues in multi-blocking queue
    ArgParser::ArgRecord arg_list[] = {
        {"--mt-queue", "MT-safe queue to use for events from other threads",
          &mtQueue, ArgParser::STRING },
        {"--multi-mt-queues", "#sub-queues in multi-blocking queue",
         &subQueues, ArgParser::INTEGER},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Setup the MT-queue for this process to use
    if (mtQueue == "single-blocking") {
        incomingEvents = new SingleBlockingMTQueue<std::mutex>();
    } else if (mtQueue == "single-blocking-sl") {
        incomingEvents = new SingleBlockingMTQueue<muse::SpinLock>();
    } else if (mtQueue == "multi-blocking") {
        incomingEvents = new MultiBlockingMTQueue<std::mutex>(subQueues);
    } else if (mtQueue == "multi-blocking-sl") {
        incomingEvents = new MultiBlockingMTQueue<muse::SpinLock>(subQueues);
    } else if (mtQueue == "multi-non-blocking") {
        incomingEvents = new MultiNonBlockingMTQueue(subQueues);
    } else {
        // Invalid mt-queue name.
        throw std::runtime_error("Invalid value for --mt-queue argument" \
                                 "(muse be: single-blocking");        
    }
    std::cout << "mtQueue set to: " << mtQueue << std::endl;
    // Let base class process consume other arguments as appropriate
    Simulation::parseCommandLineArgs(argc, argv);
}

void
MultiThreadedSimulation::preStartInit() {
    // First let the base class do the necessary setup
    Simulation::preStartInit();
    // Now override the GVT manager's rank with thread-based rank
    ASSERT(gvtManager != NULL);
    gvtManager->setThreadedRank(globalThreadID);
}

int
MultiThreadedSimulation::processMpiMsgs() {
    ASSERT(simMgr != NULL);
    ASSERT(simMgr != this);  // Ensure no recursion
    // sub-threads delegate pumping MPI messages to the manager
    // thread.  This is necessary because only manager has pointers to
    // the list of threads so that events can be added to ther
    // incoming event queue(s).
    return simMgr->processMpiMsgs();
}

void
MultiThreadedSimulation::reportLocalStatistics(std::ostream& os) {
    os << "#Deallocs cleared/call : "   << deallocsPerCall
       << "\nAvg sharedQ size       : " << shrQevtCount
       << "\n#processing of sharedQ : " << shrQcheckCount
       << "\nCPU & Numa node used   : " << cpuID
       << " [numa: "                    << getNumaNodeOfCpu(cpuID) << "]"
       << std::endl;
    // Get stats for the thread-local NUMA memory manager.
#if USE_NUMA == 1
    os << numaStats;
#endif
}

#endif
