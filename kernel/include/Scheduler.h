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
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include "HashMap.h"
#include "Agent.h"
#include "AgentPQ.h"

BEGIN_NAMESPACE(muse);
     
class Scheduler {
    friend class Simulation;
public:
    /** \brief Default Constructor

        Does nothing.
    */
    Scheduler();

    /** \brief Destructor

        Does nothing, as the constructor does nothing
    */
    ~Scheduler();

    /** \brief Schedule the given event
        
        Only used by the Simulation kernel. Users of MUSE API should
        not touch this function.  Rollback checks are done at this
        level.

        After the appropriate processing has occured, the Event will
        be placed into the appropraite Agent's Event Priority Queue.
        
        \param[in] e Event to be scheduled

        \return True if the Event was scheduled successfully
    */
    bool scheduleEvent(Event *e);

    /** \brief Instruct the current 'top' Agent to process its Events

        Only used by the Simulation kernel. Users of MUSE API should
        not touch this function.

        The 'top' Agent currently in the Fibonacci heap will be
        instructed to process all events at the current Simulation
        Time.
        
        \return True if the chosen agent had events to process.
    */
    bool processNextAgentEvents();

    /** \brief Add the specified Agent to the Scheduler

        Adds the agent to the scheduler. This happens in the
        Simulation::registerAgent method.  Only used by the Simulation
        kernel. Users of MUSE API should not touch this function.

        \param[in] agent Agent to be added to the Scheduler
        
        \return True if the agent was added to the scheduler.
    */
    bool addAgentToScheduler(Agent *agent);

    /** \brief Determine the timestamp of the next top-most event in
        the scheduler.

        This method can be used to determine the timestamp (aka
        Event::receiveTime) associated with the next event to be
        executed on the heap.  If the heap is empty, then this method
        returns INFINITY.

        \return The timestamp of the next event to be executed.
    */
    Time getNextEventTime();	

    /** \brief Update the specified Agent's key to the specified time

        This method instructions the Scheduler's internal AgentPQ to
        change the Agent's key to the specified time, and then perform
        the necessary heap fixup.

        \param[in] pointer Pointer to the AgentPQ's internal structure
        for this Agent

        \param[in] uTime The time to update the specified Agent's key
        to
    */
    void updateKey(void* pointer, Time uTime);

private:
    /** \brief Handle a rollback, if necessary
        
        This method checks if the specified event occurs before the
        agent's LVT, and performs a rollback if it does.

        \param[in] e The event to check
        
        \param[in,out] agent The agent to check against

        \return True if a rollback occured
    */
    bool checkAndHandleRollback(const Event* e, Agent* agent);

    /** \brief Perform processing of future anti-messages

        If we receive an anti-message that does not cause rollbacks,
        then currently-scheduled events that are invalidated by this
        anti-message must be removed from the given agent's event
        queue.

        \param[in] e The anti-message causing cancellations
        
        \param[in,out] agent The agent whose event queue needs to be cleaned
    */
    void handleFutureAntiMessage(const Event* e, Agent* agent);
  
protected: 
    /** The agentMap is used to quickly match AgentID to agent
        pointers in the scheduler.
    */
    AgentIDAgentPointerMap agentMap;
  
    /** The agentPQ is a fibonacci heap data structure, and used for
        scheduling the agents.
    */
    AgentPQ agentPQ;
};

END_NAMESPACE(muse);

#endif
