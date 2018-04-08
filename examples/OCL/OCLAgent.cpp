#ifndef MUSE_OCLAGENT_CPP
#define MUSE_OCLAGENT_CPP
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "OCLAgent.h"
#include <ctime>
using namespace std;
BEGIN_NAMESPACE(muse);

// The epidemic constants 

OCLAgent::OCLAgent(AgentID id, oclState* state, bool ocl, float stp, int compartmentNum, bool useODE):
    Agent(id, state), myState (state), myID(id), useOCL(ocl), step(stp), compartments(compartmentNum), ode(useODE) {
    kernel = NULL;
    fibHeapPtr = NULL;
}

void OCLAgent::setLVT(Time newLVT) { 
    lvt= newLVT; 
}
            
void OCLAgent::setKernel(oclSimulation* sim) { 
    kernel = sim; 
}

State* OCLAgent::getState() { 
    return myState; 
}

void OCLAgent::executeTask(const EventContainer& events, bool& runOCL){
      for (const Event* event : events) {

        }
      // check if using OCL kernel code
      if(useOCL){
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
        for(int i = 0; i < compartments; i++){
           myState->values[i] = values[i];
         }
        for(int i = 0; i < compartments; i++){
//              cout << values[i] << ", ";
         }
//           cout << "\n";
      }
      // create new event for next time step
      Event* event = new Event(myID, getLVT()+1);
      scheduleEvent(event);
}

void OCLAgent::executeTask(const EventContainer& events){
    return;
}

void OCLAgent::processNextEvents(muse::EventContainer& events, bool& runOCL) { 
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

void OCLAgent::seir(const float xl[4], float xln[4]) {
    // values for the disease being modeled
    float MU = 0.011;
    float A = 0.5;
    float V = 0.3333;
    float R0 = 3;
    float B  = (R0 * ((MU+A)*(MU+V)) / A);
    float N = xl[0] + xl[1] + xl[2] + xl[3];
    // represents change from compartment to compartment based on constants above
    xln[0] = (MU * N) - (MU * xl[0]) - (B * xl[2] * xl[0] / N);
    xln[1] = (B * xl[2] * xl[0] / N) - (A * xl[1]);
    xln[2] = (A * xl[1]) - ((V + MU) * xl[2]);
    xln[3] = (V * xl[2]) - (MU * xl[3]);
}

void OCLAgent::synth(const float xl[4], float xln[4]) {
    // values for the disease being modeled
    float A = 0.0005f;
    float B  = 0.00004f;
    // represents change from compartment to compartment 
    // for experimenting synthetic epidemic
    for(int i = 1; i < compartments; i++){
        xln[i] = (xl[i-1] * A) - (B * xl[i]);
    }
    xln[0] = -xln[1];
}

void OCLAgent::nextODE(float* xl) {
    const float h = step;
    // Use runge-kutta 4th order method here.
    float k1[compartments], k2[compartments], k3[compartments], k4[compartments], xlt[compartments];
    // Compute k1
    for(float j = 0; j < 1; j += step){
        seir(xl, k1);
        // Compute k2 values using k1.
        for(int i = 0; i < compartments; i++){
            xlt[i] = xl[i] + k1[i] * h / 2;
        }
        synth(xlt, k2);
        // Compute k3 values using k2.
        for(int i = 0; i < compartments; i++){
            xlt[i] = xl[i] + k2[i] * h / 2;
        }
        synth(xlt, k3);
        // Compute k4 values using k3.
        for(int i = 0; i < compartments; i++){
            xlt[i] = xl[i] + k3[i] * h;
        }
        synth(xlt, k4);

        // Compute the new set of values.
        for(int i = 0; i < compartments; i++){
            xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;
        }
    }
}

void OCLAgent::nextSSA(float* cv){
    float EventChanges[compartments-1][compartments];
    for(int i = 0; i < compartments-1; i++){
        for(int j = 0; j < compartments; j++){
            if(i==j){
                EventChanges[i][j] = -1;
            }
            else if(j-i == 1){
                EventChanges[i][j] = 1;
            }else{
                EventChanges[i][j] = 0;
            }

        }
    }
    const float rates = .1;
    for(int i = 0; i < (int)1/step; i++){
        for(int i = 0; (i < compartments-1); i++) { 
            for(int j = 0; j < compartments; j++){
                float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
                float scale = rates*step*r;
                cv[j] = cv[j] + (EventChanges[i][j] * scale); 
            }
        }
    }
}

void OCLAgent::finalize(){
    return;
}
        
void OCLAgent::initialize() throw (std::exception){
        // Generate event for this agent at time step 1
        Event* event = new Event(myID, 1);
        scheduleEvent(event);
}

END_NAMESPACE(muse);
#endif