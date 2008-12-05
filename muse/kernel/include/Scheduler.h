#ifndef _MUSE_SCHEDULER_H_
#define _MUSE_SCHEDULER_H_

#include "Event.h"
#include "DataTypes.h"
//#include "Simulation.h"
#include <queue>
#include <map>

using namespace std;
using namespace muse;

//compares delivery times. puts the one with the smaller
//ahead.

class eventComp
{
public:
  eventComp(){}
  bool operator() ( Event *lhs,  Event *rhs) const
  {
     return (lhs->getReceiveTime() > rhs->getReceiveTime());
  }
};

typedef priority_queue <Event*, vector<Event*>, eventComp> EventPQ;
typedef map<AgentID, EventPQ*> ScheduleMap;

class Scheduler {

public:
	Scheduler();
	~Scheduler();
	
	bool scheduleEvent( Event *);
	bool scheduleEvents( EventContainer *);
	EventContainer* getNextEvents(const AgentID &);
    bool addAgentToScheduler(const AgentID &);
 
private:
    ScheduleMap schedule;
};

#endif