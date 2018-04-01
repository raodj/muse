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
const float beta = 1, mu = 5e-4, psi= 0.1;
const int compartments = 4;
bool mustSaveState;
Time lvt;
int numCommittedEvents;
int numProcessedEvents;
int numSchedules;
List<Event*> inputQueue;

OCLAgent::OCLAgent(AgentID id, oclState* state, bool ocl, float stp):
    Agent(id, state), myState (state), myID(id), useOCL(ocl), step(stp) {
    kernel = NULL;
    fibHeapPtr = NULL;
}

OCLAgent::~OCLAgent(){
}

void OCLAgent::setLVT(Time newLVT) { lvt= newLVT; }
            
void OCLAgent::setKernel(oclSimulation* sim) { kernel = sim; }

Time OCLAgent::getLVT() { return lvt; }

State* OCLAgent::getState() { return myState; }

void OCLAgent::executeTask(const EventContainer& events, bool& runOCL){
      for (const Event* event : events) {
//         std::cout << "Processing: " << event << std::endl;
        myState->susceptible -= 1;//event.susceptible;
        myState->exposed     += 1;//event.exposed;
        myState->infected   += 0;//event.infected;
        myState->recovered   += 0;//event.recovered;
    }
      //check if using OCL kernel code
      if(useOCL){
        runOCL = ((myState->exposed > 0) || (myState->infected > 0));
      }else{
          //not using ocl run equations for agent immediately
          float values[4];
           values[0] = myState->susceptible;
           values[1] = myState->exposed;
           values[2] = myState->infected;
           values[3] = myState->recovered;
           nextODE(values, step);
           myState->susceptible = values[0];
           myState->exposed = values[1];
           myState->infected = values[2];
           myState->recovered = values[3];
      }
      //create new event for next time step
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
    //values for the disease being modeled
    float MU = 0.011;
    float A = 0.5;
    float V = 0.3333;
    float R0 = 3;
    float B  = (R0 * ((MU+A)*(MU+V)) / A);
    float N = xl[0] + xl[1] + xl[2] + xl[3];
    //represents change from compartment to compartment based on constants above
    xln[0] = (MU * N) - (MU * xl[0]) - (B * xl[2] * xl[0] / N);
    xln[1] = (B * xl[2] * xl[0] / N) - (A * xl[1]);
    xln[2] = (A * xl[1]) - ((V + MU) * xl[2]);
    xln[3] = (V * xl[2]) - (MU * xl[3]);
}

void OCLAgent::nextODE(float* xl, float step) {
    const float h = step;
    // Use runge-kutta 4th order method here.
    float k1[4], k2[4], k3[4], k4[4], xlt[4];
    // Compute k1
    for(float i = 0; i < 1; i += step){
        seir(xl, k1);
        // Compute k2 values using k1.
        xlt[0] = xl[0] + k1[0] * h / 2;
        xlt[1] = xl[1] + k1[1] * h / 2;
        xlt[2] = xl[2] + k1[2] * h / 2;
        xlt[3] = xl[3] + k1[3] * h / 2;
        seir(xlt, k2);
        // Compute k3 values using k2.
        xlt[0] = xl[0] + k2[0] * h / 2;
        xlt[1] = xl[1] + k2[1] * h / 2;
        xlt[2] = xl[2] + k2[2] * h / 2;
        xlt[3] = xl[3] + k2[3] * h / 2;
        seir(xlt, k3);
        // Compute k4 values using k3.
        xlt[0] = xl[0] + k3[0] * h;
        xlt[1] = xl[1] + k3[1] * h;
        xlt[2] = xl[2] + k3[2] * h;
        xlt[3] = xl[3] + k3[3] * h;
        seir(xlt, k4);

        // Compute the new set of values.
        xl[0] = xl[0] + (k1[0] + 2*k2[0] + 2*k3[0] + k4[0]) * h / 6;
        xl[1] = xl[1] + (k1[1] + 2*k2[1] + 2*k3[1] + k4[1]) * h / 6;
        xl[2] = xl[2] + (k1[2] + 2*k2[2] + 2*k3[2] + k4[2]) * h / 6;
        xl[3] = xl[3] + (k1[3] + 2*k2[3] + 2*k3[3] + k4[3]) * h / 6;
    }
}

void OCLAgent::finalize(){
    return;
}
        
void OCLAgent::initialize() throw (std::exception){
        //Generate event for this agent at time step 1
        Event* event = new Event(myID, 1);
        scheduleEvent(event);
}

END_NAMESPACE(muse);
#endif