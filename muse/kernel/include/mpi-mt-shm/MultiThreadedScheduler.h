#ifndef MUSE_MULTI_THREADED_SCHEDULER_H
#define MUSE_MULTI_THREADED_SCHEDULER_H

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

#include <mutex>
#include "Scheduler.h"
#include "ArgParser.h"
#include "ThreeTierSkipMTQueue.h"

BEGIN_NAMESPACE(muse);

// Some forward declarations

/** Version of the muse scheduler that supports multi threading 
    
    Supports multi threading and can thus be shared between multiple 
    threads, each of which can operate on it concurrently.
*/
class MultiThreadedScheduler : public Scheduler { 
    friend class MultiThreadedShmSimulationManager; // only manager initializes
public:
    /** \brief Default Constructor

        Does not have a specific task to perform and is merely present
        as a place holder for future extensions.
    */
    MultiThreadedScheduler();

    /** \brief Destructor

        Does nothing, as the constructor does nothing.
    */
    virtual ~MultiThreadedScheduler();
    
    /** \brief Instruct the current 'top' Agent to process its Events

        To be thread safe, must update simLGVT and return if events were
        processed at the same time.
        
        Only used by the Simulation kernel. Users of MUSE API should
        not touch this function.
        
        \param[out] simLGVT - updated to the new LGVT value for the sim thread
        
        \return True if the chosen agent had events to process.
    */
    virtual bool processNextAgentEvents(Time& simLGVT);
    
    /** \brief Schedule the given event in a thread safe way
        
        This method operates very similar to Scheduler::scheduleEvent()
        
        Only used by the Simulation kernel. Users of MUSE API should
        not touch this function.  Rollback checks are done at this
        level.

        After the appropriate processing has occured, the Event will
        be placed into the appropraite Agent's Event Priority Queue.
        
        \param[in] e Event to be scheduled

        \return True if the Event was scheduled successfully
    */
    virtual bool scheduleEvent(Event *e);

protected:
    
    /** \brief Complete initialization of the MT Scheduler.
      
        This allows the scheduler to set up the thread safe pending event set
        queue.

        Once the scheduler instance is created by
        Simulation::initialize() method, it must be full initialized.
        This includes processing any command-line arguments to further
        configure the scheduler.  The base class method currently does
        not perform any operation.

        \note This method may throw an exeption if errors occur.
        
        \param[in] rank The MPI rank of the process associated with
        this scheduler.

        \parma[in] numProcesses The total number of parallel processes
        in the simulation.
        
	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.
    */
    virtual void initialize(int rank, int numProcesses, int& argc,
                            char* argv[]) override;

private:
    
    /**
     * Reference to the agentPQ that includes the multi threaded aspects
     * of EventQueue.
     * 
     * muse::Scheduler also has agentPQ which must be set and be the same
     * as this reference.
     */
    EventQueueMT *mtAgentPQ;
    
};

END_NAMESPACE(muse);

#endif
