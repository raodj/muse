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
// Authors: Meseret Gebre       gebremr@muohio.edu
//         
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include <map>
#include "Agent.h"
#include "f_heap.h"

BEGIN_NAMESPACE(muse)
     
class Scheduler {
  
public:

  /** The ctor method.
   */
  Scheduler();

  /** The dtor method.
   */
  ~Scheduler();

  /** The scheduleEvent method.
      Only used by the Simulation kernel. Users of MUSE API should not touch this function.
      Check for rollback is done at this level.
      
      @param event pointer of the event to be scheduled.
   */
  bool scheduleEvent(Event *);

  /** The processNextAgentEvents method.
      Only used by the Simulation kernel. Users of MUSE API should not touch this function.
      Uses a fibonacci heapfor scheduling the agents. The agent witht he smallest event timestamp to process
      is chosen.

      @return bool, True if the chosen agent had events to process.
   */
  bool processNextAgentEvents();

  /** The addAgentToScheduler method.
      Adds the agent to the scheduler. This happend in the Simulation::registerAgent method.
      Only used by the Simulation kernel. Users of MUSE API should not touch this function.

      @param pointer to the agent.
      @return bool, True if the agent was added to the scheduler.
   */
  bool addAgentToScheduler(Agent *);

  /** The getNextEventTime method.
      Determine the timestamp of the next top-most event in the
      scheduler.

      This method can be used to determine the timestamp (aka
      Event::receiveTime) assocaited with the next event to be
      executed on the heap.  If the heap is empty, then this method
      returns INFINITY.

      @return The timestamp of the next event to be executed.
    */
  Time getNextEventTime() const;
	
 
 protected:

  /** The agentMap is used to quickly match AgentID to agent pointers in the scheduler.
   */
  typedef map<AgentID, Agent*> AgentMap;
  AgentMap agentMap;
  
  /** The agent_pq is a fibonacci heap data structure, and used for scheduling the agents.
      This is used in the processNextAgentEvents method.
   */
  typedef class boost::fibonacci_heap<Agent* , Agent::agentComp> AgentPQ;
  AgentPQ agent_pq;
};

END_NAMESPACE(muse)
#endif
