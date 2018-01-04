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



}