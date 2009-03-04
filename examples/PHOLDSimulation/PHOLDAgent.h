/* 
 * File:   PHOLDAgent.h
 * Author: Meseret Gebre
 *
 * Created on January 15, 2009, 11:04 PM
 */

#ifndef _PHOLDAgent_H
#define	_PHOLDAgent_H

#include "Agent.h"
#include "State.h"
using namespace muse;

class PHOLDAgent : public Agent {

public:
    PHOLDAgent(AgentID , State *,int x, int y, int n, int d);

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);

    void finalize();

	int X,Y,N,Delay;
};

#endif	/* _PHOLDAgent_H */

