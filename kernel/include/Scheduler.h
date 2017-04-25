#ifndef MUSE_SCHEDULER_H
#define MUSE_SCHEDULER_H

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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include "HashMap.h"
#include "Agent.h"
#include "AgentPQ.h"
#include "LadderQueue.h"
#include "TwoTierLadderQueue.h"
#include "HeapEventQueue.h"
#include "BinomialHeapEventQueue.h"
#include "TwoTierHeapEventQueue.h"
#include "ThreeTierHeapEventQueue.h"
#include "TwoTierHeapOfVectorsEventQueue.h"

BEGIN_NAMESPACE(muse);
     
class Scheduler {
    friend class Simulation;
public:
    /** \brief Default Constructor

        Does not have a specific task to perform and is merely present
        as a place holder for future extensions.
    */
    Scheduler();

    /** \brief Destructor

        Does nothing, as the constructor does nothing.
    */
    virtual ~Scheduler();

    /** \brief Schedule the given event
        
        Only used by the Simulation kernel. Users of MUSE API should
        not touch this function.  Rollback checks are done at this
        level.

        After the appropriate processing has occured, the Event will
        be placed into the appropraite Agent's Event Priority Queue.
        
        \param[in] e Event to be scheduled

        \return True if the Event was scheduled successfully
    */
    virtual bool scheduleEvent(Event *e);

    /** \brief Instruct the current 'top' Agent to process its Events

        Only used by the Simulation kernel. Users of MUSE API should
        not touch this function.

        The 'top' Agent currently in the Fibonacci heap will be
        instructed to process all events at the current Simulation
        Time.
        
        \return True if the chosen agent had events to process.
    */
    virtual bool processNextAgentEvents();

    /** \brief Remove the specified Agent from the Scheduler

        Removes the agent to the scheduler. This happens in the
        Simulation::finalize() method.  Only used by the Simulation
        kernel. Users of MUSE API should not touch this function.

        \param[in] agent Agent to be removed from the Scheduler.  The
        agent is also removed from the scheduler queue.  The scheduler
        queue is the one that actually has the brunt of the work to be
        done.
        
        \return True if the agent was successfully removed from the
        scheduler.  This method returns false if the agent was not
        found in the scheduler's internal tables.
    */
    virtual bool removeAgentFromScheduler(Agent *agent);

    /** \brief Add the specified Agent to the Scheduler

        Adds the agent to the scheduler. This happens in the
        Simulation::registerAgent method.  Only used by the Simulation
        kernel. Users of MUSE API should not touch this function.

        \param[in] agent Agent to be added to the Scheduler
        
        \return True if the agent was added to the scheduler.
    */
    virtual bool addAgentToScheduler(Agent *agent);
    
    /** \brief Determine the timestamp of the next top-most event in
        the scheduler.

        This method can be used to determine the timestamp (aka
        Event::receiveTime) associated with the next event to be
        executed on the heap.  If the heap is empty, then this method
        returns INFINITY.

        \return The timestamp of the next event to be executed.
    */
    virtual Time getNextEventTime();	

    /** \brief Update the specified Agent's key to the specified time

        This method instructions the Scheduler's internal AgentPQ to
        change the Agent's key to the specified time, and then perform
        the necessary heap fixup.

        \param[in] pointer Pointer to the AgentPQ's internal structure
        for this Agent

        \param[in] uTime The time to update the specified Agent's key
        to
    */
    virtual void updateKey(void* pointer, Time uTime);

    /** The collectGarbage method.
        
        This method is called when it is safe to garbage collect for a
	given Global Virtual Time (GVT).  Currently, the base class
	method does not have any specific operation to perform.  It is
	merely present to provide a consistent API to the various
	subsystems constituting MUSE.

        \param gvt, this is the GVT time that is calculated by GVTManager.
    */
    virtual void garbageCollect(const Time& gvt) { UNUSED_PARAM(gvt); }

    /** Method invoked just before the core simulation starts to run.

        This method is invoked after initialization and all the agents
        have been registered and they are about to be initialized
        (from Simulation::start() method).  The base class method does
        not perform any specific task and is merely present a place
        holder.

        \parma[in] startTime The logical starting time of the
        simulation.
    */
    virtual void start(const Time& startTime) { UNUSED_PARAM(startTime); }

    /** Method invoked just after the core simulation is complete.

        This method is invoked after all the agents have been
        finalized and the simulation has successfully completed.  The
        base class method does not perform any specific task and is
        merely present a place holder.
    */
    virtual void stop() {}

    /** Method to report aggregate statistics.

        This method is invoked at the end of simulation after all
        agents on this rank have been finalized.  This method can
        report any aggregate statistics from the scheduler. The
        statistics must be written to the supplied output stream.

        \note The base class method simply calls reportStats on the
        schedule queue.
        
        \param[out] os The output stream to which the statistics are
        to be written.
    */
    virtual void reportStats(std::ostream& os);
    
        
    /** Print full contents of scheduler queue to given output stream.

        This is a convenience method that is used primarily for
        troubleshooting purposes.  This method is expected to print
        the complete set of events in the event queue to the given
        output stream.  The format of the output is dependent of the
        the input queue used by the scheduler.

        \param[out] os The output stream to which the contents of the
        scheduler queue are to be written.
    */
    virtual void prettyPrint(std::ostream& os) const;
    
protected:
    /** \brief Handle a rollback, if necessary
        
        This method checks if the specified event occurs before the
        agent's LVT, and performs a rollback if it does.

        \param[in] e The event to check
        
        \param[in,out] agent The agent to check against

        \return True if a rollback occured
    */
    virtual bool checkAndHandleRollback(const Event* e, Agent* agent);

    /** \brief Perform processing of future anti-messages

        If we receive an anti-message that does not cause rollbacks,
        then currently-scheduled events that are invalidated by this
        anti-message must be removed from the given agent's event
        queue.

        \param[in] e The anti-message causing cancellations
        
        \param[in,out] agent The agent whose event queue needs to be cleaned
    */
    virtual void handleFutureAntiMessage(const Event* e, Agent* agent);

    /** \brief Complete initialization of the Scheduler.

        Once the scheduler instance is created by
        Simulation::initialize() method, it must be full initialized.
        This includes processing any command-line arguments to further
        configure the scheduler.  The base class method currently does
        not perform any operation.

        \note This method may throw an exeption if errors occur.
        
        \param[in] rank The MPI rank of the process associated with
        this scheduler.

        \parma[in] numProcesses The total number of parallel processes
        in the simulation.
        
	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.
    */
    virtual void initialize(int rank, int numProcesses, int& argc, char* argv[])
        throw (std::exception);

    /** Check if an event is within the stipulated time window.

        This method is a convenience method that is used to determine
        if one of a set of concurrent events falls within acceptable
        time window.  Time windows are used to avoid over-optimism in
        some simulations.

        \note The event is one of a set of events that will be
        processed if this method returns true.  All of the concurrent
        events are destined for the agent indicated by
        event->getReceiverAgentID() method.

        \param[in] agent The agent to which the set of concurrent
        events are to be scheduled.
        
        \param[in] event One of a set of concurrent events passed-in
        to check if the event's receive time falls within acceptable
        time window (if any).

        \return If this method returns true, then the events are
        processed.  Otherwise the scheduler performs no futher
        operations.
    */
    bool withinTimeWindow(muse::Agent* agent,
                          const muse::Event* const event) const;

    /** Obtain the time window (if any) that has been set.

        This method returns the time window that has been set for this
        scheduler.

        \return If this method returns 0, then a time window <u>has
        not been set</u>.  Valid time windows are greater than zero.
    */
    inline Time getTimeWindow() const { return timeWindow; }
    
private:
    /** The agentMap is used to quickly match AgentID to agent
        pointers in the scheduler.
    */
    AgentIDAgentPointerMap agentMap;
  
    /** The agentPQ is a fibonacci heap data structure, and used for
        scheduling the agents.
    */
    EventQueue *agentPQ;

    /** The time window set for controlling over-optimism.

        The default value is 0 to indicate no time window has been set
        (zero width time windows are invalid).  The time window can be
        set via --time-window command-line argument.
    */
    muse::Time timeWindow;
};

END_NAMESPACE(muse);

#endif
