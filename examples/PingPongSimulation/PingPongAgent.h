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

class PingPongAgent : public Agent {

public:
    PingPongAgent(AgentID &, State *);

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);

    void finalize();

};

#endif	/* _PINGPONGAGENT_H */

