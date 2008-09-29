#ifndef _MUSE_SCHEDULER_H_
#include "../include/Scheduler.h"
#endif


Scheduler::Scheduler(){}

EventContainer* Scheduler::getNextEvents(){return NULL;}

bool Scheduler::reset(){return false;}

bool Scheduler::sceduleEvent(Event * e){return false;}

bool Scheduler::sceduleEvents(EventContainer * events){return false;}

Scheduler::~Scheduler(){}