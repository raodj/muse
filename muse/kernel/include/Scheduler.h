#ifndef _MUSE_SCHEDULER_H_
#define _MUSE_SCHEDULER_H_

//#include "Event.h"
#include "DataTypes.h"
#include <queue>
#include <map>
#include "Agent.h"

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

    typedef priority_queue <Agent*, vector<Agent*>, Agent::agentComp > AgentPQ;

    AgentPQ agent_pq;
};

END_NAMESPACE(muse)
#endif
