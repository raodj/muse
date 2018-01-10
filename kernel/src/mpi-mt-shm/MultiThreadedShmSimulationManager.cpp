#ifndef MUSE_MULTI_THREADED_SHM_SIMUALTION_MANAGER_CPP
#define MUSE_MULTI_THREADED_SHM_SIMUALTION_MANAGER_CPP

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

#include "config.h"
#if USE_NUMA == 1
#include <numa.h>
#endif

#include <thread>
#include <algorithm>
#include "mpi-mt-shm/MultiThreadedShmSimulationManager.h"
#include "mpi-mt-shm/MultiThreadedShmCommunicator.h"
#include "GVTMessage.h"
#include "ArgParser.h"
#include "EventQueue.h"
#include "Scheduler.h"

// Switch to muse namespace to streamline code
using namespace muse;

MultiThreadedShmSimulationManager::MultiThreadedShmSimulationManager()
    : MultiThreadedShmSimulation(this) {
    // Nothing much to be done for now as base class does all the
    // necessary work.
}

MultiThreadedShmSimulationManager::~MultiThreadedShmSimulationManager() {
    // Necessary clean-up is done in the finalize() method to enable
    // running multiple simulations.
}

void
MultiThreadedShmSimulationManager::initialize(int& argc, char* argv[],
                                           bool initMPI)
    throw (std::exception) {
    // First, parse out the number of threads and if direct event
    // exchange is to be used.
    bool noNuma = (USE_NUMA == 1) ? false : true;
    ArgParser::ArgRecord arg_list[] = {
        { "--threads-per-node", "Number of threads to start per node/process",
          &threadsPerNode, ArgParser::INTEGER},
        { "--use-shared-events", "Share events between threads on node",
          &doShareEvents, ArgParser::BOOLEAN},
        { "--dealloc-thresh", "Desired % (0.01 to 1.0) of reclaiming shared-events",
          &deallocThresh, ArgParser::DOUBLE},
#ifdef USE_NUMA        
        {"--no-numa", "Disable use of NUMA-aware memory management",
         &noNuma, ArgParser::BOOLEAN},
#endif        
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and extract the threads-per-node
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    ASSERT( threadsPerNode > 0 );
    if ((deallocThresh  <= 0) || (deallocThresh > 1.0)) {
        std::cerr << "Invalid value for --dealloc-thresh. Value must be "
            "> 0 and <= 1\n";
        abort();
    }
    // Setup the global/static flag in EventQueue if we would like to
    // directly share events between threads.
    EventQueue::setUsingSharedEvents(doShareEvents);
    // Setup the NUMA mode of operation based on command-line arguments.
    EventRecycler::NumaSetting numaMode = EventRecycler::NUMA_NONE;
    if (!noNuma) {
        // If NUMA is enabled, then setup default mode depending on
        // whether we are sharing events or not.
        numaMode = (doShareEvents ? EventRecycler::NUMA_RECEIVER :
                    EventRecycler::NUMA_SENDER);
        // Enable per-thread NUMA-aware memory manager for the main
        // thread.
        StateRecycler::setup(true, numa_preferred());
    }
    // Next, initialize the communicator shared by multiple threads
    MultiThreadedShmCommunicator* mtc =
        new MultiThreadedShmCommunicator(this, threadsPerNode);
    // Let comm-manager use command-line arguments to configure itself.
    mtc->initialize(argc, argv, initMPI);
    setCommManager(mtc);  // set comm-manager for 'this' object.
    // Setup shared scheduler object
    // TODO (deperomm): Have this be based on command args?
    MultiThreadedScheduler* mts = new MultiThreadedScheduler();
    mts->initialize(myID, numberOfProcesses, argc, argv);
    setMTScheduler(mts);
    // Now create the necessary number of threads and initialize them.
    // To repeatedly and consistently initialize each thread class,
    // the command-line arguments are saved and restored.
    std::vector<char*> cmdArgs(argv, argv + argc);
    // First initialize & setup this class that runs as thread zero
    MultiThreadedShmSimulation::initialize(argc, argv, initMPI);
    // Only the manager explicitly initializes parent class to hijack streams
    Simulation::initialize(argc, argv, initMPI);
    // Setup our global thread ID
    globalThreadID = myID * threadsPerNode;
    // Initialize the barrier in MultiThreadedShmSimulation used for
    // coordinating the number of threads being used.
    threadBarrier.setThreadCount(threadsPerNode);
    // Create the necessary number of threads.
    createThreads(threadsPerNode, mtc, mts, cmdArgs);
    // Finally, let the base-class perform generic initialization
    muse::Simulation::initialize(argc, argv, initMPI);
    // Enable/disable NUMA-aware memory management.
    EventRecycler::setupNUMA(mtc, numaIDofThread, numaMode);
}

void
MultiThreadedShmSimulationManager::createThreads(const int threadCount,
                                              MultiThreadedShmCommunicator* mtc,
                                              MultiThreadedScheduler* mts,
                                              std::vector<char*> cmdArgs) {
    // Get the list of CPUs to be used.
    const std::vector<int> cpuList = getAvailableCPUs();
    ASSERT(!cpuList.empty());
    // If we have more threads than CPU's report a warning
    if ((int) cpuList.size() < threadCount) {
        std::cout << "Warning: More threads " << threadCount
                  << " than available cores: "
                  << cpuList.size() << std::endl;
    }
    // Re-size the numaIDs list to accommodate local thread information
    numaIDofThread.resize(threadCount);
    // Add this class as thread zero to the list of threads.
    threads.push_back(this);
    cpuID = cpuList.at(0);
    // Setup NUMA node information for this thread/CPU.
    numaIDofThread[0] = getNumaNodeOfCpu(cpuID);
    // Create the other thread classes.
    for (int thrID = 1; (thrID < threadCount); thrID++) {
        const int globalThrID = (myID * threadCount) + thrID;
        const int cpuNum      = cpuList.at(thrID % cpuList.size());
        MultiThreadedShmSimulation* tsm =
            new MultiThreadedShmSimulation(this, thrID, globalThrID,
                                        threadCount, doShareEvents, cpuNum);
        // Setup NUMA node information for this thread/CPU.
        numaIDofThread[thrID] = getNumaNodeOfCpu(cpuNum);
        // Setup the pointer to shared comm-manager and scheduler
        tsm->setCommManager(mtc);
        tsm->setScheduler(mts);
        // Setup command-line arguments from a copy to preserve original
        int argc = cmdArgs.size();
        std::vector<char*> cmdArgsCopy = cmdArgs;
        char** argv = cmdArgsCopy.data();
        // Let the thread initialize itself based on parameters.
        tsm->initialize(argc, argv, false);
        // Add the newly created thread to the list
        threads.push_back(tsm);
    }
    for (int thr = 0; (thr < threadCount); thr++) {
        std::cout << "Thread #" << thr << ": CPU=" << cpuList.at(thr)
                  << ", NUMA node: " << numaIDofThread.at(thr) << std::endl;
    }
}

std::vector<int>
MultiThreadedShmSimulationManager::getAvailableCPUs() const {
    std::vector<int> cpuList;  // Available CPUs for pinning
    cpu_set_t cpuset;          // cores from pthread.
    CPU_ZERO(&cpuset);         // Initialize with zeros.
    int err = 0;               // Error code (if any)
    if ((err = pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t),
                                      &cpuset)) != 0) {
        std::cerr << "Error in getCPUAffinity(): " << err << std::endl;
    } else {
        // Got CPU bit-map. Convert to list to make processing easier.
        for (int i = 0; (i < CPU_SETSIZE); i++) {
            if (CPU_ISSET(i, &cpuset)) {
                cpuList.push_back(i);
            }
        }
    }
    return cpuList;
}

bool
MultiThreadedShmSimulationManager::registerAgent(Agent* agent, const int threadRank = -1) {
	UNUSED_PARAM(threadRank);
    ASSERT(agent != NULL);

	// Simply register the agent, since the scheduler is shared between threads anyway
    MultiThreadedShmSimulation::registerAgent(agent);
}

void
MultiThreadedShmSimulationManager::preStartInit() {
    ASSERT( commManager != NULL );
    // Let base class method do initial setup
    MultiThreadedShmSimulation::preStartInit();
    // Have all the threads do their pre-start init
    for (size_t thrIdx = 1; (thrIdx < threads.size()); thrIdx++) {
        ASSERT(threads[thrIdx] != NULL);
		// MATT: Shared GVTManager? At the moment each thread has its own ****************************
        threads[thrIdx]->preStartInit();
    }
    // Finally let the comm-manager finish exchanging list of agents
    // on different MPI processes which is needed to enable exchange
    // of events.
    mtCommMgr->registerAllAgents();
    // Setup initial capacity on container to hold incoming MPI events
    mpiEvents.reserve(maxMpiMsgThresh * 2);
}

void
MultiThreadedShmSimulationManager::start() {
    // Finish all the setup for all the threads prior to starting simulation.
    preStartInit();
    // Now spwan threadsPerNode - 1 threads (this is thread #0 already)
    ASSERT( threadsPerNode > 0 );
    std::vector<std::thread> thrList;
    for (int thrIdx = 1; (thrIdx < threadsPerNode); thrIdx++) {
        // Setup most up to date parameter/options
        threads[thrIdx]->setStartTime(startTime);
        threads[thrIdx]->setStopTime(endTime);
        // Spin-up the thread.
        thrList.push_back(std::thread(&MultiThreadedShmSimulation::simulate,
                                      threads[thrIdx]));
    }
    // Now perform the simulation tasks for this thread #0
    simulate();
    // Now that the simulation is done, wind-up the threads
    std::for_each(thrList.begin(), thrList.end(),
                  std::mem_fn(&std::thread::join));
    // Wait for all the parallel processes to complete the main
    // simulation loop.
    MPI_BARRIER();    
}

void
MultiThreadedShmSimulationManager::finalize(bool stopMPI, bool delCommMgr) {
    // First let all the sub-threads finalize
    for (size_t thrIdx = 1; (thrIdx < threads.size()); thrIdx++) {
        threads[thrIdx]->finalize(false, false);
    }
    // Next, finalize this thread #0
    MultiThreadedShmSimulation::finalize(stopMPI, delCommMgr);
    // Free-up pending events from sub-threads to be cleaned up. There
    // maybe some to be cleaned as Agent's input/output queues could
    // be holding onto the last few events.  These events are added to
    // the thread #0's pendingDeallocs list by other threads before
    // they join thread #0 in simulate() method in this class.
    ASSERT(EventRecycler::pendingDeallocs.empty());
    ASSERT(EventRecycler::Recycler.empty());    
    // Finally, get rid of all the thread helper classes as they are no
    // longer needed.  The thread #0 will be deleted in
    // Simulation::finalizeSimulation() method if user requests it.
    for (size_t thrIdx = 1; (thrIdx < threads.size()); thrIdx++) {
        delete threads[thrIdx];
    }
    threads.clear();  // Clear out the vector as a sanity check.
}

#endif
