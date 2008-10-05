#ifndef _MUSE_EVENT_H_
#include "Event.h"
#endif

using muse::AgentID;
using muse::Time;

muse::Event::Event(AgentID &id, Time &deliveryTime): sentTime(0), deliveryTime(0){}

muse::Event::~Event(){}
