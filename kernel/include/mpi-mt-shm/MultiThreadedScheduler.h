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

BEGIN_NAMESPACE(muse);

// Some forward declarations

/** Version of the muse scheduler that supports multi threading 
    
    Supports multi threading and can thus be shared between multiple 
    threads, each of which can operate on it concurrently.
*/
class MultiThreadedScheduler : public Scheduler { 
    
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
};

END_NAMESPACE(muse);

#endif