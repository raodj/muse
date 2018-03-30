#ifndef MUSE_OCLAGENT_H
#define MUSE_OCLAGENT_H
#include "../../kernel/include/EventRecycler.h"
#include "Agent.h"
#include <oclSimulation.h>
#include "oclState.h"

BEGIN_NAMESPACE(muse);

//forward declarations
class oclSimulation;
class oclState;
class OCLAgent : public Agent {
    friend class oclSimulation;
    public:
        
       /** The constructor.
        
        \note once constructed MUSE will handle deleting the state
        pointer.  State can only be allocated in the heap.
	
        \param id the ID that this agent will take on
        
        \param agentState pointer to the state that has been allocated
        in the heap
    */
        OCLAgent(AgentID id, oclState* state, bool ocl);
        
        ~OCLAgent();
        
        /** The executeTask method.
     
        This method is invoked only when the agent has some events to
        process. The events that this agent must process at a given
        time step is all passed in one single shot to this method.
         
         This method is overridden to pass a boolean value that checks if 
         the agent needs to have equations run with the ocl kernel
        
        \param events The set of concurrent (events at the same time)
        events that this method should process.
        */    
        void executeTask(const EventContainer& events, bool& runOCL);
        
        void executeTask(const EventContainer& events) override;

        /** The processNextEvents method.
        
        Only for use by the Scheduler. This is used to get the next
	set of events to be processed by this agent. The Scheduler
	will call this method, when it is time for processing.
        
        */
        void processNextEvents(muse::EventContainer& events, bool& runOCL);

        /*
         * Helper function for nextODE
         * function runs seir equations with values
         * specific to the disease being modeled.
         */
        void seir(const float xl[4], float xln[4]);
        /*
         Makes a step with the ODE equations -
         * runs Runge Kutta fourth order equations
         * to advance the agent one time step in the simulation
         * 
         * xl is the values of the current state and they are updated
         * within the method
         */
        void nextODE(float* xl, float step);
        
        /** The setLVT method.
        
        \param newLVT the new LVT
        */
        void setLVT(Time newLVT);
        
        /** Setup the simulation kernel to be used by this agent.

        This method is invoked from the Simulation kernel (or its
        derived classes) when an agent is registered to set the
        pointer to the kernel that logically owns this agent.

        \param sim The simulation/kernel that is agent must use for
        scheduling events etc.  The pointer is never null.
        */
        void setKernel(oclSimulation* sim);
        
        /** The getLVT method.

            This will return the agent's Local Virtual Time.

            \return The LVT -- the time of the last processed event
        */
        Time getLVT();

        /** The getState method.
        This will return the current state of the agent.

        \return the current state pointer to the agent's state.
        */
        State* getState();
        
        /*
        The finalize method.
     
        This method is invoked once the simulation has finished
        processing all events and is ending.  The core simulation
        engine invokes this method. This method may perform any final
        clean up or displaying results.
         */
        void finalize() override;
        
        /** The initialize method.
         Function runs before the core simulation loop. 
         Creates an event for the agent
         */
        void initialize() throw (std::exception) override;
        
        //id for the agent
        AgentID myID;
        
        //State of the agent
        oclState* myState;
        
        //set when agent is created, dictates if running ocl code
        bool useOCL;

};
END_NAMESPACE(muse);
#endif