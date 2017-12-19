/* 
 * File:   RoundRobinAgent.h
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:04 PM
 */

#ifndef _LonelyAgent_H
#define	_LonelyAgent_H

#include "Agent.h"
#include "State.h"
using namespace muse;

class LonelyAgent : public Agent {

public:
    LonelyAgent(AgentID &, State *);

    void initialize() throw (std::exception) override;

    void executeTask(const EventContainer& events) override;

    void finalize() override;

};

#endif	/* _LonelyAgent_H */

