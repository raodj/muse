#ifndef MUSE_OCLAGENT_H
#define MUSE_OCLAGENT_H

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

#include "EventRecycler.h"
#include "Agent.h"
#include "oclSimulation.h"
#include "oclState.h"
#include <string>

BEGIN_NAMESPACE(muse);

// Use real for all state data so users can redefine real as anything needed
typedef float real;

// forward declarations
class oclSimulation;
class oclState;
class oclAgent : public Agent {
    friend class oclSimulation;
    friend class oclScheduler;
    friend class synthAgent;
    friend class synthSimulation;
    /*
     * Functions inside public are the functions that can be defined by the user
     * in order to change the way that the decision to run opencl kernel code is made
     * as well as how to process and execute tasks
     */
    public:
        bool mustSaveState;
        Time lvt;
        int numCommittedEvents;
        int numProcessedEvents;
        int numSchedules;
        List<Event*> inputQueue;
        // State of the agent
        oclState* myState;
        oclSimulation* kernel;

        /** The constructor.
        
        \note once constructed MUSE will handle deleting the state
        pointer.  State can only be allocated in the heap.
	
        \param id the ID that this agent will take on
        
        \param agentState pointer to the state that has been allocated
        in the heap
    */
        oclAgent(AgentID id, oclState* state);

        /** The executeTask method.
     
        This method is invoked only when the agent has some events to
        process. The events that this agent must process at a given
        time step is all passed in one single shot to this method.
         
         This method is overridden to pass a boolean value that checks if 
         the agent needs to have equations run with the ocl kernel
        
        \param events The set of concurrent (events at the same time)
        events that this method should process.
        */    
        virtual void executeTask(const EventContainer& events, bool& runOCL);

        void executeTask(const EventContainer& events) override;

         /** Setup the simulation kernel to be used by this agent.

        This method is invoked from the Simulation kernel (or its
        derived classes) when an agent is registered to set the
        pointer to the kernel that logically owns this agent.

        \param sim The simulation/kernel that is agent must use for
        scheduling events etc.  The pointer is never null.
        */
        virtual void setKernel(oclSimulation* sim);

        /*
         Makes a step with the ODE equations -
         * runs Runge Kutta fourth order equations
         * to advance the agent one time step in the simulation
         * 
         * xl is the values of the current state and they are updated
         * within the method
         */
        virtual void nextODE(real* xl) = 0;

        /*
         Makes a step with the SSA equations -
         * runs Gillespie with Tau Leaping optimization
         * to advance the agent one time step in the simulation
         * 
         * cv is the values of the current state and they are updated
         * within the method
         */
        virtual void nextSSA(real* cv) = 0;

        /*
          * Returns the kernel code for this type of agent
          * Called from the oclSimulation class
          * Allows for multiple agent types
          */
        virtual std::string getKernel() = 0;

        /*
         *  The initialize method.
         * Function runs before the core simulation loop. 
         * Creates an event for the agent
         */
        void initialize() throw(std::exception) override;

    protected:
        /** The processNextEvents method.
        
        Only for use by the Scheduler. This is used to get the next
	set of events to be processed by this agent. The Scheduler
	will call this method, when it is time for processing.
        
        */
        virtual void processNextEvents(muse::EventContainer& events, bool& runOCL);

        /** The setLVT method.        
        \param newLVT the new LVT
        */
        virtual void setLVT(Time newLVT);


        /** The getState method.
        This will return the current state of the agent.

        \return the current state pointer to the agent's state.
        */
        virtual State* getState();

        /*
        The finalize method.
     
        This method is invoked once the simulation has finished
        processing all events and is ending.  The core simulation
        engine invokes this method. This method may perform any final
        clean up or displaying results.
         */
        void finalize() override;

        /** The getLVT method.
        
        This will return the agent's Local Virtual Time.
	
        \return The LVT -- the time of the last processed event
        */
        inline Time getLVT() const { return lvt; }
};
END_NAMESPACE(muse);
#endif
