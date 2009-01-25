/* 
 * File:   ClockAgent.h
 * Author: gebremr
 *
 * Created on December 10, 2008, 11:29 PM
 */

#ifndef _CLOCKAGENT_H
#define	_CLOCKAGENT_H
#include "Agent.h"
#include "ClockState.h"
using namespace muse;

class ClockAgent : public Agent {

public:
    ClockAgent(AgentID &, ClockState *);

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);
    
    void finalize();
    
};

#endif	/* _CLOCKAGENT_H */

