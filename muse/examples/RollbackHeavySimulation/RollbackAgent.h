/* 
 * File:   PingPongAgent.h
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:04 PM
 */

#ifndef _PINPONGGAGENT_H
#define	_PINPONGGAGENT_H

#include "Agent.h"
#include "State.h"
using namespace muse;

class RollbackAgent : public Agent {

public:
    RollbackAgent(AgentID &, State *, int );

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);

    void finalize();

    int max_agents;
};

#endif	/* _PINGPONGAGENT_H */

