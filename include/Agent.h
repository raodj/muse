
#include "f_heap.h"

#ifndef MUSE_AGENT_H
#define MUSE_AGENT_H

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
// Authors: Meseret Gebre       gebremr@muohio.edu
//          Dhananjai M. Rao    raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <iostream>
#include <exception>
#include "DataTypes.h"
#include "State.h"
#include <list>
#include "Event.h"
#include "f_heap.h"

BEGIN_NAMESPACE(muse) //begin namespace declaration


/** The base class for all agents in a simulation.
 
    <p>This class represents the base class from which all
    user-defined simulation agents must be derived. This class
    provides several methods that expose the core functionality of a
    muse simulation. In addition, the user is expected to implement
    some of the methods in this class. Refer to the documentation
    associated with each method for details on the functionality and
    requirements of the variuos methods in this class.</p>
        
    <p>To create an agent for your simulation, make sure you derive
    from this Agent Class, Also be sure to override all methods that
    are declared virtual.</p>
*/
class Agent {

    //lets declare the Simulation class a friend!
    friend class Simulation;
    //lets declare the Scheduler class a friend!
    friend class Scheduler;

    //class boost::fibonacci_heap<Event* , eventComp>;
    
public:
    /** The initialize method.
     
        This method is invoked whenever just when a simulation
        commences. The core simulation engine invokes this
        mehtod. This mehtod may perform any agent specific
        initialization activites that may be necessary. This includes
        opening new streams for performing I/O.
        
        @note Whenever a new agent is created, the dervied child class
        should override this method.

        @throw std::exception This method throws an exception when
        errors occur.
    */
    virtual void initialize() throw (std::exception);
        
    /** The finalize method.
     
        This method is invoked once the simulation has finished
        processing all events and is ending.  The core simulation
        engine invokes this method. This method may perform any final
        clean up or displaying results, basic I/O.
     
        @note Whenever a new agent is created, the derived child class
        should override this method.
    */
    virtual void finalize();
    
    /** The executeTask method.
     
        This method is invoked only when the agent has some events to
        process. The events that this agent must process at a given
        time step is all passed in one single shot to this method.
     
        @note That this method must not modify the events passed in.
        
        @note Whenever a new agent is implemented by extending this
        base class, the derived class should override this method.
        
        @param events The set of concurrent (events at the same time)
        events that this method should process.
        
        @see EventContainer
    */
    virtual void executeTask(const EventContainer * events);

    //------------Provided by muse----below-----------------//
    
    /** The scheduleEvent method.
     
        This method is used to schedule an event to be processed by any
        agent.  

     * IMPORTANT - once event is scheduled MUSE will take full ownership, thus it will handle
     *             deleting the memory.
        @param e this is an Event object and will not be modified by this
        method.
        
        @return bool this will let the client know if the operation was a
        success.
        
        @see Event()
    */
    bool scheduleEvent(Event * e);

    /** The scheduleEvent method.

     *   This method is used to schedule more than one event to be processed by any
     *   agent.
     *
     * IMPORTANT - once event is scheduled MUSE will take full ownership, thus it will handle
     *             deleting the memory.

        @param e this is an Event object and will not be modified by this
        method.

        @return bool this will let the client know if the operation was a
        success.

        @see Event()
     *  @see DataTypes.h
     *  @see EventContainer
    */
    //bool scheduleEvents(EventContainer * events);
    
    /** The getAgentID method returns a pointer to AgentID struct.
        
        This method is used to return a pointer to a AgentID struct
        which contains the id of the agent. Once pointer is obtained,
        simple using the command AgentID.id would extract the actual
        id, but for all purpose the client should have no reason to do
        this, unless the id is required for I/O purposes. All method
        that require will only take of type AgentID.
        
        Note: All agents must register with the muse::Simulator which
        in returns assigns an AgentID. This AgentID is what this
        method will be returning a pointer to.
        
        Note: AgentID that is returned should not be modified in anyway.
        
        @return AgentID 
        @see AgentID
        
    */
    const AgentID& getAgentID();
        
    /**The getLVT method.
     * This will return the agent's Local Virtual Time.
     * If you want to know the time in term of an agent, this is what you
     * want to call.
     *
     * @return Time , basically the time of the last processed event
     */
    Time getLVT();
	
    /** The constructor.
        
        The ctor is not supposed to be overriden.
        @todo make sure that the above statement is a accurate statement.
    */
    explicit Agent(AgentID &, State *);
    
    /** The destructor.
     
        The dtor is not supposed to be overriden.
        @todo make sure that the above statement is a accurate statement.
    */
   virtual ~Agent();


    /**The cloneState method.
     * Simply returns a copy of the passed in state. Every state should be able to
     * clone itself.
     *
     *IMPORTANT - MUSE will handle disposing of this State pointer only if MUSE kernel
     *            makes this call. However, if for some reason the user calls for a clone
     *            then it is important to properly dispose the pointer when operation is complete.
     *
     *@param State pointer, this is the state to be cloned.
     *@return State, pointer to new state.
     */
    State* cloneState(State *);

    /**The setState method.
     * Currently waiting to be approaved for implementation.
     * Agents will be able to use different states throughout the simulation
     * via this method. Note, that the state thats passed in the constructure
     * does not have to be the only state. You can For example slowly increase
     * the information stored in the state as simulation runs.
     *
     *IMPORTANT - this method uses the assignment operator, so be sure you properly
     *            overload the assignment operator for the given state classes.
     *
     *@param State reference, this is the new state that will be set.
     *@return bool, true if the process was a success.
     */
    void setState(State *);

private:


    /**The processNextEvents method.
     * This is used for in house and SHOULD NEVER be used by users. Only for the Simulation kernel.
     * This is used to get the next set of events to be processed by this agent. The scheduler 
     * will call this method, when it is time for processing.
     *
     *@return bool, true if there were events to process at the time of the method call
     */
    bool processNextEvents();

protected:

    //the following four methods are for rollback recovery
    void doRollbackRecovery(Event * );
    void doStepOne(Event * );
    void doStepTwo();
    void doStepThree(Event * );


    class agentComp{
    public:
        agentComp(){}
       inline bool operator()(const Agent *lhs, const Agent *rhs) const
        {
           //cout << "Here" <<endl;
            if (lhs->eventPQ.empty() && rhs->eventPQ.empty()) {
               return true;
            }
            if (lhs->eventPQ.empty() && !rhs->eventPQ.empty()) {
                return true;
            }
            if (rhs->eventPQ.empty() && !lhs->eventPQ.empty()) {
                return false;
            }
            Event *lhs_event = lhs->eventPQ.top();
            Event *rhs_event = rhs->eventPQ.top();
            Time lhs_time    = lhs_event->getReceiveTime();
            Time rhs_time    = rhs_event->getReceiveTime();
            return (lhs_time > rhs_time);
        }//end operator()
    };
    //compares delivery times. puts the one with the smaller
    //ahead.
    class eventComp
    {
    public:
      eventComp(){}
      inline bool operator() ( Event *lhs,  Event *rhs) const
      {
         Time lhs_time = lhs->getReceiveTime(); //hack to remove warning during compile time
         return (lhs_time > rhs->getReceiveTime());
      }

    };
     
     typedef boost::fibonacci_heap<Event* , eventComp> EventPQ;
    //typedef priority_queue <Event*, vector<Event*>, eventComp > EventPQ;
    
    /** The AgentID type myID.
     
        This is inialized when the agent is registered to a simulator.
        Only one way to access this, use the getAgentID method.
      
        @see getAgentID()
    */
    AgentID myID;

    /** The AgentID type myID.

    This is inialized when the agent is registered to a simulator.
    Only one way to access this, use the getAgentID method.

    @see getAgentID()
    */
    Time LVT;

    list<State*> stateQueue;
    list<Event*> outputQueue;
    list<Event*> inputQueue;
    State* myState;
    EventPQ eventPQ;

};
END_NAMESPACE(muse)//end namespace declaration
#endif
 
