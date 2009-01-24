#ifndef _MUSE_SCHEDULER_H_
#define _MUSE_SCHEDULER_H_

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
	Scheduler();
	~Scheduler();
	
	bool scheduleEvent(Event *);
	//bool scheduleEvents( EventContainer *);
	bool processNextAgentEvents();
        bool addAgentToScheduler(Agent *);
 
    //this will be used to figure out which agents should process events
   // bool operator()(const Agent *lhs, const Agent *rhs) const;
private:
    typedef map<AgentID, Agent*> AgentMap;
    AgentMap agentMap;
    AgentContainer allAgents;

    //typedef priority_queue <Agent*, vector<Agent*>, Agent::agentComp > AgentPQ;
    typedef boost::fibonacci_heap<Agent* , Agent::agentComp> AgentPQ;
    AgentPQ agent_pq;
};

END_NAMESPACE(muse)
#endif
