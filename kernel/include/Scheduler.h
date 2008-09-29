#ifndef _MUSE_SCHEDULER_H_
#define _MUSE_SCHEDULER_H_

#include "../../include/Event.h"


using namespace muse;
class Scheduler {

public:
	Scheduler();
	~Scheduler();
	
	bool reset();
	bool sceduleEvent(Event *);
	bool sceduleEvents(EventContainer *);
	EventContainer* getNextEvents();
	
	
private:
	
};
#endif