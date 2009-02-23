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
#include <list>
#include "DataTypes.h"
#include "Event.h"
#include "State.h"

BEGIN_NAMESPACE(boost);
template <typename T, typename Comp >
class fibonacci_heap;

//template<class T>
//class intrusive_ptr;
END_NAMESPACE(boost);

BEGIN_NAMESPACE(muse); //begin namespace declaration

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

public:
    /** The eventComp class.
        Compares delivery times. puts the one with the smaller ahead.
    */
    class eventComp{
    public:
        eventComp(){}
        inline bool operator() ( Event *lhs,  Event *rhs) const{
            Time lhs_time = lhs->getReceiveTime(); //hack to remove warning during compile time
            return (lhs_time > rhs->getReceiveTime());
        }
    };
    
private:
    //forward declare
    //class State;
    
public:
    /** The initialize method.
        
    This method is invoked just when a simulation
    commences. The core simulation engine invokes this
    mehtod. This mehtod may perform any agent specific
    initialization activites that may be necessary. This includes
    opening new streams for performing I/O.
    
    @note Whenever a new agent is created, the derived child class
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

    @note once event is scheduled MUSE will take full ownership, thus it will handle
    deleting the memory.
		  
    @param pointer to the event to schedule.
    @return bool True if the event was correctly scheduled.
    @see Event()
    */
    bool scheduleEvent(Event * e);

    
    /** The getAgentID method.
        @return reference to this agent's id
        @see AgentID
    */
    inline const AgentID& getAgentID() const { return myID; }
        
    /** The getLVT method.
        This will return the agent's Local Virtual Time.
        If you want to know the time in term of an agent, this is what you
        want to call.
     
        @return Time , basically the time of the last processed event
    */
    inline const Time& getLVT() const { return LVT;}
	
    /** The ctor.
        @note once constructed MUSE will handle deleting the state pointer.
        State can only be allocated in the heap.
	
        @param the AgentID that this agent will take on.
        @param pointer to the state that has been allocated in the heap.
    */
    explicit Agent(AgentID , State *);
    
    /** The destructor.
     
    The dtor is not supposed to be overriden.
    @todo make sure that the above statement is a accurate statement.
    */
    virtual ~Agent();


    /** The cloneState method.
        Simply returns a copy of the passed in state. Every state should be able to
        clone itself.
	
        @note MUSE will handle disposing of this State pointer only if MUSE kernel
        makes this call. However, if for some reason the user calls for a clone
        then it is important to properly dispose the pointer when operation is complete.
	
        @param State pointer, this is the state to be cloned.
        @return State, pointer to new state.
    */
    virtual State* cloneState(State *);

    /** The setState method.
        Agents will be able to use different states throughout the simulation
        via this method. Note, that the state thats passed in the constructure
        does not have to be the only state. You can For example slowly increase
        the information stored in the state as simulation runs.

        @note this method uses the assignment operator, so be sure you properly
        overload the assignment operator for the given state classes.
	
        @param State reference, this is the new state that will be set.
        @return bool, true if the process was a success.
	@see State
    */
    void setState(State *);

protected:

    
    /** The processNextEvents method.
        This is used for in house and SHOULD NEVER be used by users. Only for the Scheduler.
        This is used to get the next set of events to be processed by this agent. The scheduler 
        will call this method, when it is time for processing.
     
        @return bool, true if there were events to process at the time of the method call
    */
    bool processNextEvents();

    /** The cleanStateQueue method
        Used by the Simulation kernel to delete remaining events in the state queue.
    */
    void cleanStateQueue();

    /** The cleanInputQueue method
        Used by the Simulation kernel to delete remaining events in the input queue.
    */
    void cleanInputQueue();

    /** The cleanOutputQueue method
        Used by the Simulation kernel to delete remaining events in the output queue.
    */
    void cleanOutputQueue();


    /** The collectGarbage method.
        When this method is called, garbage collection takes place.
	Since the three queues are all double linked list, we simply
	start from the front and remove all elements that have a time
	smaller than the gvt (global virtual time).

        @param gvt, this is the GVT time that is calculated by GVTManager.
        @see GVTManager
        @see Time
     */
    void garbageCollect(const Time gvt);

    
    /** The doRollbackRecovery method.  This method is called by the
        scheduler class, when a rollback is detected.

        @note Only should be called by the Scheduler class.
	@param pointer to the event that caused the rollback aka straggler
	event
    */
    void doRollbackRecovery(Event * );

    /** The doRestorationPhase method.  This is the first step of the
        rollback recovery process.  In this method we first search the
        state queue and find a state with a timestamp that is smaller
        than that of the straggler event's receive time. we also
        delete all state with timestamp greater than straggler event's
        time

        @note Only should be called by the doRollbackRecovery method.
        @param pointer to the event that caused the rollback aka straggler event
    */
    void doRestorationPhase(Event * );

    /** The doCancellationPhaseOutputQueue method.  This is the second
        step of the rollback recovery process.  In this method we
        start prunning the output queue. Events with sent time greater
        than straggler event's time are removed from the output
        queue. Also anti-messages are sent for the earliest sent time
        after the straggler event's time.
	
        @note Only should be called by the doRollbackRecovery method.
        
    */
    void doCancellationPhaseOutputQueue();

    /** The doCancellationPhaseInputQueue method.
        This is the third and final step of the rollback recovery process.
        In this method we start prunning the input queue. Events
        with receive time greater than straggler event's time are removed
        from the input queue. 
	
        @note Only should be called by the doRollbackRecovery method.
        @param pointer to the event that caused the rollback aka straggler event
	
    */
    void doCancellationPhaseInputQueue(Event * );

    /** The agentComp class.
        This is used by the scheduler class to figure out which agent to schedule next.
    */
    class agentComp{
    public:
        agentComp(){}
        bool operator()(const Agent *lhs, const Agent *rhs) const;
    };

   
    /** The EventPQ type.
        This is data structure that houses the events to be processed by this agent.
        This is a fibonacci heap! Uses the eventComp as a comparator.

        @see f_heap()
        @see eventComp()
        @see Event()
    */
    typedef boost::fibonacci_heap<Event* , eventComp> EventPQ;

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

    /** The double linked lise stateQueue.
        Houses a sorted list of state pointers. Needed incase of rollbacks
        @see State()
    */
    list<State*> stateQueue;


    /** The intrusive pointer for events. This way we dont have to worry
	about what container has what reference and we can be leak free
	for Event memory.
     */
    //typedef boost::intrusive_ptr<Event> SharedEventPointer;
    
    /** The double linked lise outputQueue.
        Houses a sorted list of event pointers. Needed incase of rollbacks
        @see Event()
    */
    list<Event*> outputQueue;

    /** The double linked lise inputQueue.
        Houses a sorted list of event pointers. Needed incase of rollbacks
        @see Event()
    */
    list<Event*> inputQueue;

    /** The State type myState.
        This is a auto pointer to the current state of the Agent.
	Using smart pointer allows worry free use of the state class and
	doesn't leak memory.
        @see State()
    */
    auto_ptr<State> myState;

   

    /** The EventPQ type eventPQ.
        This is access to the fibonacci heap data structure that houses or events to process.
        @see f_heap()
        @see EventPQ
    */
    EventPQ *eventPQ;

   
};
END_NAMESPACE(muse);//end namespace declaration
#endif
 
