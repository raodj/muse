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
#include <deque>
#include <cmath>
#include "DataTypes.h"
#include "Event.h"
#include "State.h"
#include "oSimStream.h"

/** Use this macro to compare to Time values safely
 */
#define TIME_EQUALS(t1,t2)(fabs(t1-t2)<1e-8)

// Forward declaration for insertion operator for Event
extern std::ostream& operator<<(ostream&, const muse::Agent&);

BEGIN_NAMESPACE(muse);

//forward declare here
class BinaryHeapWrapper;
template<typename T, typename Compare>
class BinaryHeap;
class EventQueue;
class Tier2Entry;
class EventComp;
class TwoTierHeapAdapter;

/** \typedef std::deque<T> List<T>

    \brief Data structure to define the list of events and states
    managed by each agent.  This alias provides a convenient approach
    to quickly change the underlying data structure used in the code
    with minor modification (and lots of testing).
*/
template<typename T>
using List = std::deque<T>;

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
    friend std::ostream& ::operator<<(ostream&, const muse::Agent&);
    friend class Simulation;
    friend class Scheduler;
    friend class AgentPQ;
    friend class TwoTierHeapEventQueue;
    friend class ThreeTierHeapEventQueue;
    friend class HeapOfVectorsEventQueue;
public:    
    /** enum for return Time.
        when agent ask for time via getTime()
        user will be able to pass in TimeType and get
        LVT,LGVT, or GVT. default will be LVT (Local Virtual Time)
    */
    enum TimeType {LVT, LGVT, GVT};
    
    /** The initialize method.
        
        This method is invoked just when a simulation
        commences. The core simulation engine invokes this
        mehtod. This mehtod may perform any agent specific
        initialization activites that may be necessary. This includes
        opening new streams for performing I/O.
    
        \note Whenever a new agent is created, the derived child class
        should override this method.

        \throw std::exception This method throws an exception when
        errors occur.
    */
    virtual void initialize() throw (std::exception) = 0;
    
    /** The finalize method.
     
        This method is invoked once the simulation has finished
        processing all events and is ending.  The core simulation
        engine invokes this method. This method may perform any final
        clean up or displaying results, basic I/O.
     
        \note Whenever a new agent is created, the derived child class
        should override this method.
    */
    virtual void finalize() = 0;
    
    /** The executeTask method.
     
        This method is invoked only when the agent has some events to
        process. The events that this agent must process at a given
        time step is all passed in one single shot to this method.
     
        \note That this method must not modify the events passed in.
        
        \note Whenever a new agent is implemented by extending this
        base class, the derived class should override this method.
        
        \param events The set of concurrent (events at the same time)
        events that this method should process.
        
        \see EventContainer
    */
    virtual void executeTask(const EventContainer* events) = 0;

    //------------Provided by muse----below-----------------//
    
    /** The scheduleEvent method.
     
        This method is used to schedule an event to be processed by any
        agent.  

        \note once event is scheduled MUSE will take full ownership,
        thus it will handle deleting the memory.
		  
        \param pointer to the event to schedule.
        \return bool True if the event was correctly scheduled.
        \see Event()
    */
    bool scheduleEvent(Event* e);
    
    /** The getAgentID method.
        \return reference to this agent's id
        \see AgentID
    */
    inline AgentID getAgentID() const { return myID; }

    /** The getTime method.
        This will return the Simulation Time or the GVT (global virtual time).

        \param timeType, this is of can be either Agent::lvt,
        Agent::lgvt, or Agent::gvt
        
        \return Time, Depending on the time type. See TimeType for
        what each one returns, default is LVT.
        
	\see Time
	\see TimeType
    */
    Time getTime(TimeType timeType = Agent::LVT) const;

    /** The constructor.
        
        \note once constructed MUSE will handle deleting the state
        pointer.  State can only be allocated in the heap.
	
        \param id the ID that this agent will take on
        
        \param agentState pointer to the state that has been allocated
        in the heap
    */
    explicit Agent(AgentID id, State* agentState);
    
    /** The destructor.
     
        The destructor is not supposed to be overriden.
        \todo make sure that the above statement is a accurate statement.
    */
    virtual ~Agent();

    /** The cloneState method.
        
        Simply returns a copy of the passed in state. Every state
        should be able to clone itself.
	
        \note MUSE will handle disposing of this State pointer only if
        MUSE kernel makes this call. However, if for some reason the
        user calls for a clone then it is important to properly
        dispose the pointer when operation is complete.
	
        \param state State to be cloned
        
        \return new State object that is a copy of the given state
    */
    virtual State* cloneState(State* state);

    /** The setState method.
        
        Agents will be able to use different states throughout the
        simulation via this method. Note, that the state thats passed
        in the constructure does not have to be the only state. You
        can For example slowly increase the information stored in the
        state as simulation runs.

        \note this method uses the assignment operator, so be sure you
        properly overload the assignment operator for the given state
        classes.
	
        \param State reference, this is the new state that will be
        set.
        
	\see State
    */
    void setState(State *);

    /** The registerSimStream method.
        
	If the user wants to use another SimStream just create it and
	make sure to register it with the agent you will be using it
	in.

	\note if all you want is to display out to std::cout, then
	there is already a default oSimStream called "oss", and you
	use it just like std::cout.

	\note using this method informs MUSE that you are using the
	user defined SimStream for some stream operation, please
	remember to keep the reference to the pointer you created.

	\note MUSE will not handle disposing of any registered
	SimStream. Please remember to clean up.
	
	\param newSimStream, pointer to the SimStream of your choice.
        
	\see SimStream
	\see oSimStream
    */
    void registerSimStream(SimStream* newSimStream);

    /** The getState method.
        This will return the current state of the agent.

        \return the current state pointer to the agent's state.
        \see State
    */
    inline State* getState() const { return myState; }

protected:
    /** The oSimStream type oss.
        
        This is the default provided by MUSE and when its safe will
        push all data to std::cout.
    */
    oSimStream oss;

    /** Save the current state

        This method is a wrapper to save the state of this Agent and
        all necessary Simulation Streams. It is called by
        Agent::processNextEvents() and Simulation::start() as
        needed. Deriving classes should not call this method unless
        they are working with MUSE Simulation kernel internals.  This
        method can be overrided for advanced purposes, but it is
        highly recommended to avoid doing so.
    */
    virtual void saveState();

    /** Utility method to dump running stats about this agent to an
        output stream.

        This method is typically used to generate a brief one line
        status information about the agent during simulation.  This
        method is typically called from Simulation::dumpStats() method
        to generate current, aggregate status of a given process in
        the parallel simulation.  The information logged by this
        method is typically used for troubleshooting/debugging and
        does not constitute the model information generated via
        simulation.

        \param[out] os The output stream to which the stats are to be
        written.

        \param[in] printHeader If this flag is true, then this method
        must print the column titles for the different values being
        logged in a human readable form.  Typically, the first agent
        is asked to print the header information.
    */
    virtual void dumpStats(std::ostream& os,
                           const bool printHeader = false) const;
private:
    /** The getTopTime method.

        \deprecated This method is deprecated.  Instead the
        funtionality of this method must be implemented in the
        respective EventQueue derived class(es).
        
        Simply returns the receive Time of the top event in the
        eventPQ. If eventPQ is empty then TIME_INIFINITY is
        returned. This is used heavily in AgentPQ class.
		
        \return Time, the top event recv time or TIME_INFINITY if
        eventPQ empty.
    */
    __attribute__((deprecated))
    Time getTopTime() const;
    
    /** The getNextEvents method.

        \deprecated This method is deprecated.  Instead the
        funtionality of this method must be implemented in the
        respective EventQueue derived class(es).
        
        This method is a helper that will grab the next
        set of events to be processed by this agent.
		
        \param[out] container The reference of the container into
        which events should be added.        
    */
    __attribute__((deprecated))
    void getNextEvents(EventContainer& container);

    /** The setLVT method.
        
        \param newLVT the new LVT
    */
    inline void setLVT(Time newLVT) { lvt= newLVT; }
    
    /** The getLVT method.
        
        This will return the agent's Local Virtual Time.
	
        \return The LVT -- the time of the last processed event
    */
    inline Time getLVT() const { return lvt; }
    
    /** The processNextEvents method.
        
        Only for use by the Scheduler. This is used to get the next
	set of events to be processed by this agent. The Scheduler
	will call this method, when it is time for processing.

        \param[in] events The set of events to be processed by this
        agent.
        
        \return bool, true if there were events to process at the time
        of the method call
    */
    void processNextEvents(muse::EventContainer& events);

    /** The cleanStateQueue method
        
        Used by the Simulation kernel to delete remaining events in
        the state queue.
    */
    void cleanStateQueue();

    /** The cleanInputQueue method
        
        Used by the Simulation kernel to delete remaining events in
        the input queue.
    */
    void cleanInputQueue();

    /** The cleanOutputQueue method
        
        Used by the Simulation kernel to delete remaining events in
        the output queue.
    */
    void cleanOutputQueue();

    /** The collectGarbage method.
        
        When this method is called, garbage collection takes place.
	Since the three queues are all double linked list, we simply
	start from the front and remove all elements that have a time
	smaller than the gvt (global virtual time).

        \param gvt, this is the GVT time that is calculated by GVTManager.
        
        \see GVTManager
        \see Time
    */
    void garbageCollect(const Time gvt);
    
    /** The doRollbackRecovery method.  This method is called by the
        scheduler class, when a rollback is detected.

        \note Only should be called by the Scheduler class.
        
	\param[int] stragglerEvent pointer to the event that caused
	the rollback (aka straggler event).  This pointer should not
	be NULL.

        \param[out] reschedule The queue to be used to reschedule
        events to be reprocessed due to the rollback.  These events
        are added back to the event queue associated with the
        scheduler.
    */
    void doRollbackRecovery(const Event* stragglerEvent,
                            muse::EventQueue& reschedule);

    /** The doRestorationPhase method.  This is the first step of the
        rollback recovery process.  In this method we first search the
        state queue and find a state with a timestamp that is smaller
        than that of the straggler event's receive time. we also
        delete all state with timestamp greater than straggler event's
        time

        \note Only should be called by the doRollbackRecovery method.

        \param stragglerTime, reference to the strggler event's receive time
    */
    void doRestorationPhase(const Time& stragglerTime);

    /** The doCancellationPhaseOutputQueue method.  This is the second
        step of the rollback recovery process.  In this method we
        start prunning the output queue. Events with sent time greater
        than straggler event's time are removed from the output
        queue. Also anti-messages are sent for the earliest sent time
        after the straggler event's time.
	
        \note Only should be called by the doRollbackRecovery method.
        
    */
    void doCancellationPhaseOutputQueue(const Time& restoredTime);

    /** The doCancellationPhaseInputQueue method.
        This is the third and final step of the rollback recovery process.
        In this method we start prunning the input queue. Events
        with receive time greater than straggler event's time are removed
        from the input queue. 
	
        \note Only should be called by the doRollbackRecovery method.
	
        \param[in] restoredTime, this is the time restored from the state
        
	\param[in] straggler_sender_agent_id, this is the agent that sent
	the straggler event

        \param[out] reschedule The list of events to be rescheduled
        for reprocessing due to a rollback.  The list of events to be
        rescheduled is passed to the scheduler that appropriately
        handles it.
        
	\see doRestorationPhase
	\see Time
	\see AgentID
	
    */
    void doCancellationPhaseInputQueue(const Time& restoredTime,
                                       const Event* straggler,
                                       muse::EventContainer& reschedule);

    /** The agentComp class.
        
        This is used by the scheduler class to figure out which agent
        to schedule next.
    */
    class agentComp{
    public:
        inline bool operator()(const Agent *lhs, const Agent *rhs) const;
    };

    /** The AgentID type myID.
        
        This is inialized when the agent is registered to a simulator.
        Only one way to access this, use the getAgentID method.
	
        \see getAgentID()
    */
    AgentID myID;

    /** The AgentID type myID.
        
        This is inialized when the agent is registered to a simulator.
        Only one way to access this, use the getAgentID method.
	
        \see getAgentID()
    */
    Time lvt;

    /** The list State Queue.
        
        Houses a sorted list of state object-pointers. The list is
        used to restore the state of the agent as part of rollback
        recovery process.  Since events are scheduled in receive
        timestamp order, and state saving is governed by event
        processing or LVT of an agent, the entries in this list are
        automatically sorted based on LVT of the agent.  The list is
        periodically garbage collected based on GVT.

        \note Prior to Dec 21 2016, this data structure was a
        std::list<State*> which was memory intensive.  Consequently,
        it has been changed to a std::deque instead.

        \see State()
    */
    List<State*> stateQueue;

    /** The double linked list Output Queue.
        
        Houses a sorted list of events. These events were generated by
        this Agent.  The list is used to sent anti-messages as part of
        rollback recovery process.  The list is periodically garbage
        collected based on GVT.

        \note Prior to Dec 21 2016, this data structure was a
        std::list<State*> which was memory intensive.  Consequently,
        it has been changed to a std::deque instead.
        
        \see Event()
    */
    List<Event*> outputQueue;

    /** The double linked list Input Queue.
        
        Houses a sorted list of events that has been scheduled for
        this agent.  This list is used to cancel/reprocess events as
        part of rollback recovery processing.  The list is
        periodically garbage collected based on GVT.

        \note Prior to Dec 21 2016, this data structure was a
        std::list<State*> which was memory intensive.  Consequently,
        it has been changed to a std::deque instead.
        
        \see Event()
    */
    List<Event*> inputQueue;

    /** The State type myState.
        
        This is a pointer to the current state of the Agent.  Each
        time a set of concurrent events have been processed by this
        agent, a copy of this state object is made an added to the
        state queue.

        \see State()
    */
    State* myState;
    
    std::vector<Tier2Entry>* tier2;
        
    union {
        /** The TwoTierHeapAdapter for TwoTierHeapEventQueue.
            
            This object is setup by TwoTierHeapEventQueue.  It is used
            by TwoTierHeapEventQueue to manage a binary heap data
            structure that houses events to be scheduled for this
            specific Agent.  Placing this pointer in Agent object
            helps conceptually organize events appropriately.
            
            \see TwoTierHeapEventQueue::addAgent()
        */
        TwoTierHeapAdapter* agentEventHeap;

        /** The BinaryHeap that is used by Fibonacci Heap (AgentPQ).
            
            This is access to the custom binary heap data structure
            that houses events associated with this agent.
            
            \see BinaryHeap()
        */
        BinaryHeapWrapper* eventPQ;
        
        /** The BinaryHeap type tier2eventPQ.
            
            This is access to the custom binary heap data structure that
            houses Tier2Entry objects to process.  
            
            \see BinaryHeap()
        */
        BinaryHeap<muse::Tier2Entry, EventComp>* tier2eventPQ;
    } schedRef;
    
    /** The pointer type.
        
        Pointer to the location of the agent in the fibonacci heap.
        This pointer is used only when Fibonacci heap is being used
        for scheduling.  Otherwise this list pointer is unused. 
    */
    void* fibHeapPtr;

    /** The lowest time stamp event at the end of fibonacci heap
        operations.

        This time stamp is needed to manage the position of this agent
        on the fibonacci heap.  It is updated by the fibonacci heap at
        the end of various event queue management operations.
    */
    Time oldTopTime;
    
    /** The SimStreamContainer type allSimStreams.
        
	This is used to store registered SimStream, typically are user
	defined.
    */
    SimStreamContainer allSimStreams;

    /** Flag to indicate if this agent must save state at the end of
        each event processing cycle.

        This flag by default is set to true to indicate that the agent
        must save state at the end of each event processing cycle.
        This flag is overridden by Simluation::registerAgent() class.
        It is set to false if only one MPI process is being used for
        simulation.  A user may force state saving using the
        --must-save-state command-line parameter.
    */
    bool mustSaveState;
    
    ////////////////////////////////////////////////////////
    
    /** The next few variables are used for benchmarking and reporting
        simulation statistics associated with this agent.
    */
    int numRollbacks;
    int numScheduledEvents;
    int numProcessedEvents;
    int numMPIMessages;

    /**
       Instance variable to track the number of events there were
       actually processed and not rolled-back. This instance variable
       is incremented each time an event is garbage collected (because
       garbage collected events are committed and are never
       rolled-back).
    */
    int numCommittedEvents;

    /**
       Number of times this agent was scheduled for processing events.
       This instance variable essentially tracks the raw number of
       times the executeTask() method was invoked by the Scheduler
       (consequently with rollbacks this value will be higher than the
       count in a 1 process simulation).
    */
    int numSchedules;
};

END_NAMESPACE(muse);

#endif
 
