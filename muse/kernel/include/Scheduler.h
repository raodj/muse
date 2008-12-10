#ifndef _MUSE_SCHEDULER_H_
#define _MUSE_SCHEDULER_H_

#include "Event.h"
#include "DataTypes.h"
#include <queue>
#include <map>

BEGIN_NAMESPACE(muse);


 
class Scheduler {
public:
	Scheduler();
	~Scheduler();
	
	bool scheduleEvent( Event *);
	bool scheduleEvents( EventContainer *);
	EventContainer* getNextEvents(const AgentID &);
        bool addAgentToScheduler(const AgentID &);
 
private:
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
    typedef priority_queue <Event*, vector<Event*>, eventComp > EventPQ;
    typedef map<AgentID, EventPQ*> ScheduleMap;

    ScheduleMap schedule;
};

END_NAMESPACE(muse);
#endif
