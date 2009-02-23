/* 
 * File:   RoundRobinAgent.h
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:04 PM
 */

#ifndef _RoundRobinAgent_H
#define	_RoundRobinAgent_H

#include "Agent.h"
#include "State.h"
using namespace muse;

class RoundRobinAgent : public Agent {

public:
    RoundRobinAgent(AgentID &, State *, int );

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);

    void finalize();

    int max_agents;
};

#endif	/* _RoundRobinAgent_H */

