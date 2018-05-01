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

#include "ocl/OclAgent.h"
#include <ctime>
#include <vector>
BEGIN_NAMESPACE(muse);

OclAgent::OclAgent(AgentID id, OclState* state):
    Agent(id, state), myState(state) {
}

void OclAgent::setLVT(Time newLVT) {
    lvt = newLVT;
}

void OclAgent::setKernel(OclSimulation* sim) {
    kernel = sim;
}

State* OclAgent::getState() {
    return myState;
}

void OclAgent::executeTask(const EventContainer& events) {
    return;
}

void OclAgent::processNextEvents(muse::EventContainer& events, bool& runOCL) {
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
                ASSERT(EventRecycler::getReferenceCount(curr) == 1);
                ASSERT(EventRecycler::getInputRefCount(curr)  == 1);
                // The order of decreasing reference counters is important
                EventRecycler::decreaseInputRefCount(curr);
            }
            ASSERT(EventRecycler::getReferenceCount(curr) == 1);
            ASSERT(EventRecycler::getInputRefCount(curr)  == 0);
            EventRecycler::decreaseOutputRefCount(curr);
        }
      }
}

void OclAgent::finalize() {
    return;
}

void OclAgent::initialize() throw(std::exception) {
        // Generate event for this agent at time step 1
        Event* event = new Event(myID, 1);
        scheduleEvent(event);
}

void OclAgent::executeTask(const muse::EventContainer& events, bool& runOCL) {
      for (const muse::Event* event : events) {
          // fill in event processing if needed
       }
      // check if using OCL kernel code
        if (kernel->oclAvailable) {
            // check if disease present in agent, if so
            // runOCL is true and opencl kernel will process this agent
            if (kernel->compartments >= 4) {
                runOCL = ((myState->values[1] > 0) || (myState->values[2] > 0));
            } else {
                runOCL = myState->values[1] > 0;
            }
        } else {
        // not using ocl run equations for agent immediately
        std::vector<real> value;
        for (int i = 0; i < kernel->compartments; i++) {
            value.push_back(myState->values[i]);
        }
        real* values = value.data();
        if (kernel->ode) {
          nextODE(values);
        } else {
            std::vector<std::vector<int>> v;
          nextSSA(values, v, {});
        }
        // put values back into agent
        for (int i = 0; i < kernel->compartments; i++) {
           myState->values[i] = values[i];
         }
      }
      // create new event for next time step
      if(getLVT() < kernel->getStopTime()){
        muse::Event* event = new muse::Event(myID, getLVT()+1);
        scheduleEvent(event);
      }
}

void OclAgent::nextODE(float* xl) {
    const float h = kernel->step;
    // Use runge-kutta 4th order method here.
    real k1[kernel->compartments], k2[kernel->compartments],
            k3[kernel->compartments], k4[kernel->compartments],
            xlt[kernel->compartments];
    // Compute k1
    for (real j = 0; j < 1; j += kernel->step) {
        // run seir equations
        seir(xl, k1);
        // Compute k2 values using k1.
        for (int i = 0; i < kernel->compartments; i++) {
            xlt[i] = xl[i] + k1[i] * h / 2;
        }
        // run seir equations
        seir(xlt, k2);
        // Compute k3 values using k2.
        for (int i = 0; i < kernel->compartments; i++) {
            xlt[i] = xl[i] + k2[i] * h / 2;
        }
        // run seir equations
        seir(xlt, k3);
        // Compute k4 values using k3.
        for (int i = 0; i < kernel->compartments; i++) {
            xlt[i] = xl[i] + k3[i] * h;
        }
        // run seir equations
        seir(xlt, k4);

        // Compute the new set of values.
        for (int i = 0; i < kernel->compartments; i++) {
            xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;
        }
    }
}

void OclAgent::nextSSA(real* cv, std::vector<std::vector<int>> EventChanges, std::vector<real> rates) {
    // events and rates are passed in from child class and used here
    // seed random value creation
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rnd(seed);
    // loop based upon time step
    for (int i = 0; i < static_cast<int>(1/kernel->step); i++) {
        // loop through all potential events
        for (int i = 0; i < EventChanges.size(); i++) {
            // create random value based on rate and compartment that is
            // being removed from
            std::poisson_distribution<int> pRNG(rates[i]);
            real num = static_cast<real>(pRNG(rnd)) * kernel->step;
            for (int j = 0; j < EventChanges[0].size(); j++) {
                // add that value to the current value
                cv[j] = cv[j] + (EventChanges[i][j] * num);
            }
        }
    }
}

std::string OclAgent::getKernel() {
    // Called from child class and appended to existing kernel code there
    // Create an ostream
    std::ostringstream oclKernel;
    // check if using ssa or ode
    if (!kernel->ode) {
        // loop based on time step
        oclKernel << "   for(int x = 0; x < (int)1/stp; x++){\n"
        // loop through all potential events
        "     for(int i = 0; (i < sizeof EventChanges[0] / sizeof(int)); i++) {\n"
        // using random values passed in and current state values, create random value
        "       float scale = rates[i] * ((float)random[(id + x) % 100] / 100.0f) * stp * cv[i];\n"
        "        for(int j = 0; j < sizeof EventChanges / sizeof EventChanges[0]; j++){\n"
        // add random value based on event impact on compartments
        "           cv[j+id] = cv[j+id] + (EventChanges[i][j] * scale);\n"
        "        }\n"
        "     }\n"
        "   }\n"
        "}\n";
    } else {
        oclKernel <<
        "        // Compute k1\n"
        // loop based on time step and run runge kutta 4th order equations
        "   for(int j = 0; j < (int)(1.0f/h); j++){\n"
        // run seir equations
        "   seir(xl, k1);"
        "\n"
        "        // Compute k2 values using k1.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xlt[i] = xl[i+id] + k1[i] * h / 2;\n"
        "        }\n"
        "        \n"
        // run seir equations
        "   seir(xlt, k2);"
        "\n"
        "        // Compute k3 values using k2.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xlt[i] = xl[i+id] + k2[i] * h / 2;\n"
        "        }\n"
        "\n"
        // run seir equations
        "   seir(xlt, k3);"
        "\n"
        "        // Compute k4 values using k3.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xlt[i] = xl[i+id] + k3[i] * h;\n"
        "        }\n"
        "\n"
        // run seir equations
        "   seir(xlt, k3);"
        "\n"
        "        // Compute the new set of values.\n"
        "        for (int i = 0; i < compartments; i++) {\n"
        "            xl[i+id] = xl[i+id] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;\n"
        "        }\n"
        "    }  \n"
        "}\n";
    }
    return oclKernel.str();

}

END_NAMESPACE(muse);
#endif
