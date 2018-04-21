#ifndef MUSE_OCLAGENT_CPP
#define MUSE_OCLAGENT_CPP
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
// Authors: Harrison Roth          rothhl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "oclAgent.h"
#include <ctime>

BEGIN_NAMESPACE(muse);

oclAgent::oclAgent(AgentID id, oclState* state, bool ocl, float stp, int compartmentNum, bool useODE):
    Agent(id, state), compartments(compartmentNum), ode(useODE), myState (state), useOCL(ocl), step(stp)  {
    kernel = NULL;
    fibHeapPtr = NULL;
}

void oclAgent::setLVT(Time newLVT) { 
    lvt= newLVT; 
}
            
void oclAgent::setKernel(oclSimulation* sim) { 
    kernel = sim;    
}

State* oclAgent::getState() { 
    return myState; 
}

void oclAgent::executeTask(const EventContainer& events){
    return;
}

void oclAgent::processNextEvents(muse::EventContainer& events, bool& runOCL) { 
      // change runOCL to true requires ocl code to be run
      // The events cannot be empty
      ASSERT(!events.empty());
      // Flag for -- are we directly sharing events beween threads?
      const bool usingSharedEvents = kernel->usingSharedEvents();
      // Add the events to our input queue only when state saving is
      // enabled -- that is we have more than 1 process and rollbacks
      // are possible.

      if (mustSaveState) {
        for (EventContainer::iterator curr = events.begin();
             (curr != events.end()); curr++) {
            ASSERT(!usingSharedEvents ||
                   (EventRecycler::getInputRefCount(*curr) > 0));
            inputQueue.push_back(*curr);
        }
      } else {
        // keep track number of processed events -- this would be
        // normally done during garbage collection.  However, in 1 LP
        // mode we do not add events to input queue and consequently
        // we need to track it here.
        numCommittedEvents += events.size();
      }
      DEBUG(std::cout << "Agent " << getAgentID() << " is scheduled to process "
                    << events.size() << " events at time: "
                    << events.front()->getReceiveTime()
                    << " [committed thusfar: " << numCommittedEvents << "]\n");
      ASSERT(events.front()->getReceiveTime() > getState()->timestamp);
      // Set the LVT and timestamp
      setLVT(events.front()->getReceiveTime());
      getState()->timestamp = getLVT();

      // Let the derived class actually process events that it needs to.
      // Pass boolean value to execute task to decide if ocl needs to be run on agent 
      executeTask(events, runOCL);

      DEBUG(std::cout << "Agent " << getAgentID() << " is done processing "
                    << events.size() << " events at time: "
                    << events.front()->getReceiveTime() << " [committed "
                    << "thusfar: " << numCommittedEvents << "]\n\n");
      // Increment the numProcessedEvents and numScheduled counters
      numProcessedEvents += events.size();
      numSchedules++;

      // Save the state (if needed) now that events have been processed.
      saveState();

      if (!mustSaveState) {
        // This applicable only in sequential mode. So the ASSERT
        // establishes the necessary conditions.
        ASSERT((kernel->getNumberOfProcesses() == 1) &&
               (kernel->getNumberOfThreads()   == 1));
        // Decrease reference and free-up events as we are not adding to 
        // input queue for handling rollbacks that cannot occur in this case.
        for (muse::Event* curr : events) {
            // Ensure these conditions are met in single-threaded mode
            if (usingSharedEvents) {
                ASSERT( EventRecycler::getReferenceCount(curr) == 1 );
                ASSERT( EventRecycler::getInputRefCount(curr)  == 1 );
                // The order of decreasing reference counters is important
                EventRecycler::decreaseInputRefCount(curr);
            }
            ASSERT( EventRecycler::getReferenceCount(curr) == 1 );
            ASSERT( EventRecycler::getInputRefCount(curr)  == 0 );
            EventRecycler::decreaseOutputRefCount(curr);
        }
      }
}

void oclAgent::finalize(){
    return;
}
        
void oclAgent::initialize() throw (std::exception){
        // Generate event for this agent at time step 1
        Event* event = new Event(myID, 1);
        scheduleEvent(event);
}

void oclAgent::executeTask(const muse::EventContainer& events, bool& runOCL){
    std::cout << "Executing Task" << std::endl;
      for (const muse::Event* event : events) {
          // fill in event processing if needed
       }
      // check if using OCL kernel code
      if(useOCL){
          // check if disease present in agent, if so
          // runOCL is true and opencl kernel will process this agent
        if(compartments >= 4){
            runOCL = ((myState->values[1] > 0) || (myState->values[2] > 0));
        }else{
            runOCL = myState->values[1] > 0;
        }
      }else{
        // not using ocl run equations for agent immediately
        float values[compartments];
        for(int i = 0; i < compartments; i++){
            values[i] = myState->values[i];
        }
        if(ode){
          nextODE(values);
        }else{
          nextSSA(values);
        }
        // put values back into agent
        for(int i = 0; i < compartments; i++){
           myState->values[i] = values[i];
         }
//        for(int i = 0; i < compartments; i++){
//              cout << values[i] << ", ";
//         }
//           cout << "\n";
      }
      // create new event for next time step
      muse::Event* event = new muse::Event(myID, getLVT()+1);
      scheduleEvent(event);
}

END_NAMESPACE(muse);
#endif