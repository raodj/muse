#ifndef MUSE_MULTI_THREADED_SHM_SIMUALTION_CPP
#define MUSE_MULTI_THREADED_SHM_SIMUALTION_CPP

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

#include "NumaMemoryManager.h"
#include <string>
#include "mpi-mt-shm/MultiThreadedShmSimulationManager.h"
#include "mpi-mt-shm/MultiThreadedShmCommunicator.h"
#include "mpi-mt-shm/MultiThreadedScheduler.h"
#include "SpinLock.h"
#include "GVTManager.h"
#include "GVTMessage.h"
#include "SimulationListener.h"
#include "ArgParser.h"
#include "EventAdapter.h"
#include "EventRecycler.h"
#include "StateRecycler.h"
#include "mpi-mt/RedistributionMessage.h"

// Switch to muse namespace to streamline code
using namespace muse;

// The shared barrier to facilitate coordination of threads to help
// clean-up recycled events.
SpinLockThreadBarrier MultiThreadedShmSimulation::threadBarrier;

// Shared mutex lock to allow thread safe pulling of mpi events from the wire
std::mutex MultiThreadedShmSimulation::mpiMutex;

// The static/shared list of NUMA-node IDs for each thread.  This list
// is static and is shared by multiple threads.  Entries are added by
// the derived class, in MultiThreadedSimulationManager::createThreads
std::vector<int> MultiThreadedShmSimulation::numaIDofThread;
MultiThreadedShmSimulation::MultiThreadedShmSimulation(MultiThreadedShmSimulationManager* mgr,
                                                 int thrID, int threadsPerNode,
                                                 bool usingSharedEvents, int cpuID) :
    Simulation(usingSharedEvents), threadsPerNode(threadsPerNode),
    threadID(thrID), mtCommMgr(NULL),
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
    // Nothing much to be done for now as base class does all the
    // necessary work.
}

MultiThreadedShmSimulation::~MultiThreadedShmSimulation() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

void
MultiThreadedShmSimulation::setCommManager(MultiThreadedShmCommunicator* mtc) {
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
MultiThreadedShmSimulation::setMTScheduler(MultiThreadedScheduler* sch) {
    ASSERT(sch != NULL);
    // Make sure 'scheduler' set only by this method and not the base class
    // to ensure scheduler is a MultiThreadedScheduler type. (there is
    // logic in Simulation::parseCommandLineArgs to prevent this)
    ASSERT(scheduler == NULL);
    mtScheduler = sch;
    scheduler = sch; // let the base class hold the MTscheduler
}

void
MultiThreadedShmSimulation::setGVTManager(GVTManager* gvtMan) {
    gvtManager = gvtMan;
    gvtManager->addMTSimKernel(this);
}

void
MultiThreadedShmSimulation:: initialize(int& argc, char* argv[], bool initMPI) {
    UNUSED_PARAM(initMPI);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);
    // make sure parseCommandLineArgs doens't set scheduler for MT sims
    ASSERT(scheduler == NULL);
}

void
MultiThreadedShmSimulation::finalize(bool stopMPI, bool delCommMgr) {
    // Only thread specific finalizing operations gets done here
    
    // Both of these recyclers use thread_local storage, so clear them
    EventRecycler::deleteRecycledEvents();
    StateRecycler::deleteRecycledStates();
    
    // These are thread local, so ensure they are empty on each thread
    // This also ensures there aren't any events accidently left in an
    // agent event queue
    ASSERT(EventRecycler::Recycler.empty()); 
    ASSERT(EventRecycler::pendingDeallocs.empty());
}

void
MultiThreadedShmSimulation::setCpuAffinity() const {
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
MultiThreadedShmSimulation::getNumaNodeOfCpu(const int cpu) const {
#if USE_NUMA == 1
    return numa_node_of_cpu(cpu);
#else
    UNUSED_PARAM(cpu);
    return -1;
#endif
}

// NOTE: This method is the main thread method and is called from
// multiple threads!
void
MultiThreadedShmSimulation::simulate() {
    // Setup the thread ID in our EventRecycler for initializing
    muse::EventRecycler::threadID = this->threadID;
    // Setup thread affinity basedon cpuID set for this thread.
    setCpuAffinity();
    // Always, start-up the per-thread NUMA memory manager.
    EventRecycler::startNUMA();
    // Important: wait for threads to finish setup before starting
    // event processing on all threads.
    threadBarrier.wait();
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
        // This should only be done on thread 0 to avoid race conditions
        if (threadID == 0) {
            if (--gvtTimer == 0) {
                gvtTimer = gvtDelayRate;
                // Initate another round of GVT calculations if needed.
                gvtManager->startGVTestimation();
            }
        }
        // Process a block of events received via the network
        // (eventually goes to derived manager class when
        // processMpiMsgs() method is called by the base class).
        checkProcessMpiMsgs();
        // Process the next event from the list of events managed by
        // the scheduler.
        if (!processNextEvent()) {
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;
        }
    }
    // Wait for all the threads to finish by waiting on a barrier.
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

bool
MultiThreadedShmSimulation::processNextEvent() {
    // Update lgvt to the time of the next event to be processed.
    // Let the scheduler do its task to have the agent process events
    // if possible.  If no events are processed the following method
    // call returns false.
    bool ret = mtScheduler->processNextAgentEvents(LGVT);
    // Do sanity checks.
    if (LGVT < getGVT()) {
        std::cout << "Offending event: "
                  << *scheduler->agentPQ->front() << std::endl;
        std::cout << "LGVT = " << LGVT << " is below GVT: " << getGVT()
                  << " which is serious error. Scheduled agents: \n";
        std::cout << "Rank " << myID << " Aborting.\n";
        std::cout << std::flush;
        DEBUG(logFile->close());
        abort();
    }
    return ret;    
}

//muse::Event*
//MultiThreadedShmSimulation::cloneEvent(const muse::Event* src,
//                                    const muse::AgentID receiver) const {
//    ASSERT(src != NULL);
//    ASSERT(receiver >= 0);
//    // First suitably allocate memory for the copy
//    const int evtSize = EventAdapter::getEventSize(src);    
//    char* mem;  // Initialized in if-else below.
//    if (EventRecycler::numaSetting == EventRecycler::NUMA_SENDER) {
//        // Override default memory management and allocate event on
//        // the receiver's NUMA node.
//        mem = EventRecycler::allocateNuma(EventRecycler::NUMA_RECEIVER,
//                                          evtSize, receiver);
//    } else {
//        // Use default memory management of Event.
//        mem = Event::allocate(evtSize, receiver);
//    }
//    ASSERT( mem != NULL );
//    muse::Event* const copy = reinterpret_cast<muse::Event*>(mem);
//    std::memcpy(copy, src, evtSize);
//    ASSERT(copy->getReferenceCount() > 0);
//    return copy;
//}

bool 
MultiThreadedShmSimulation::scheduleEvent(Event* e) {
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
    // check if event is on this process or a remote one
    if (commManager->isAgentLocal(myID)) {
        // Event is on this process
        return mtScheduler->scheduleEvent(e);
    } else {
        // The destination thread is on a remote process and event
        // does not need to be cloned.  However, it does need
        // inspection by GVT manager.
        gvtManager->sendRemoteEvent(e);
    }
    return true;
}

void
MultiThreadedShmSimulation::garbageCollect() {
    // Garbage collect in same fashion as base class but lock the agents
    // (since this is multi threaded and agents must be locked before modified)
    const Time gvt = getGVT();
    // First let the scheduler know it can garbage collect.
    scheduler->garbageCollect(gvt);
    for (AgentContainer::iterator it = allAgents.begin();
            (it != allAgents.end()); it++) { 
        (*it)->garbageCollect(gvt);
    }
    // Let listener know garbage collection for a given GVT value has
    // been completed.
    if (listener != NULL) {
        listener->garbageCollectionDone(gvt);
    }
    
    
#if USE_NUMA == 1
    // With NUMA we periodically need to redistribute events to avoid
    // unconstrained memory growth (depending on communication patterns)
    if (getGVT() != TIME_INFINITY) {
        MultiThreadedShmSimulationManager* const mgr =
            static_cast<MultiThreadedShmSimulationManager*>(simMgr);
        EventRecycler::numaMemMgr.redistribute(threadsPerNode, threadID,
                                               numaIDofThread[threadID], mgr);
    }
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
MultiThreadedShmSimulation::parseCommandLineArgs(int &argc, char* argv[]) {
    
    // MTSimulation arguments would be parsed here, if there were any...

    // Let base class process consume other arguments as appropriate
    Simulation::parseCommandLineArgs(argc, argv);
}

int
MultiThreadedShmSimulation::processMpiMsgs() {
    // If we have only one process then there is nothing to be done
    if (numberOfProcesses < 2) {
        return 0;  // no other process to communicate with.
    }
    // First try to lock the shared MPI mutex.  If it is not lockable,
    // then some other thread is pumping MPI messages for all
    // threads. So skip rest of the operation.
    if (!mpiMutex.try_lock()) {
        return 0;  // Some other thread is working.
    }
    // We got the lock read MPI messages if any.
    std::lock_guard<std::mutex> lock(mpiMutex, std::adopt_lock);
    // Track number of times the processMpiMsgs method is called.
    processMpiMsgCalls++;
    // An optimization trick is to try to get as many events from the
    // wire as we can. A good magic number is 100.  However this
    // number could be dynamically adapted depending on behavior of
    // the simulation.
    ASSERT(mpiEvents.empty());
    if (mtCommMgr->receiveManyEvents(mpiEvents, maxMpiMsgThresh) <= 0) {
            return 0;  // No events were obtained from MPI
    }
    // Process the incoming MPI events.
    for (Event* incoming_event : mpiEvents) {
        ASSERT(incoming_event->getReferenceCount() == 1);
        // Enqueue the incoming event to the appropriate thread.
        if (incoming_event->getSenderAgentID() == -1) {
            // This must be a GVT message.
            ASSERT(incoming_event->getReceiverAgentID() <= 0);
            ASSERT(dynamic_cast<GVTMessage*>(incoming_event) != NULL);
            processIncomingEvent(incoming_event);
        }
        else {
            // Increment input reference counter for this event to
            // balance decrement in MultiThreadedSimulation::
            // processIncomingEvents() -- this is to balance
            // expectation between locally-shared events versus events
            // received over the wire
            if (doShareEvents) {
                EventRecycler::increaseInputRefCount(doShareEvents,
                        incoming_event);
            }
            processIncomingEvent(incoming_event);
        }
    }
    // Save events received over mpi to return
    const int msgCount = mpiEvents.size();
    // Track the batch size if numMsgs is greater than zero.
    if (msgCount > 0) {
            mpiMsgBatchSize += msgCount;
    }
    // Clear out the processed events in preparation for next cycle
    mpiEvents.clear();
    // Return the number of events received.
    return msgCount;
}

void 
MultiThreadedShmSimulation::processIncomingEvent(Event* event) {
    if (event->getSenderAgentID() == -1) {
        // This must be a GVT message.
        ASSERT(event->getReceiverAgentID() <= 0);
        ASSERT(event->getReceiveTime() == -1);
        ASSERT(dynamic_cast<GVTMessage*>(event) != NULL);
        GVTMessage *msg = static_cast<GVTMessage*>(event);
        ASSERT(msg != NULL);
        gvtManager->recvGVTMessage(msg);
    }
    else if (event->getSenderAgentID() == REDISTR_MSG_SENDER) {
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
    }
    else {
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
        }
        else {
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

void
MultiThreadedShmSimulation::reportLocalStatistics(std::ostream& os) {
    os << "#Deallocs cleared/call : "   << deallocsPerCall
//       << "\nAvg sharedQ size       : " << shrQevtCount
//       << "\n#processing of sharedQ : " << shrQcheckCount
       << "\nCPU & Numa node used   : " << cpuID
       << " [numa: "                    << getNumaNodeOfCpu(cpuID) << "]"
       << std::endl;
    // Get stats for the thread-local NUMA memory manager.
#if USE_NUMA == 1
    os << numaStats;
#endif
}

#endif
