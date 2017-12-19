/* 
 * File:   ClockAgent.h
 * Author: gebremr
 *
 * Created on December 10, 2008, 11:29 PM
 */

#ifndef CLOCK_AGENT_H
#define	CLOCK_AGENT_H

#include "Agent.h"
#include "ClockState.h"


/** A straightforward agent that schedules an event to itself.
    
 */
class ClockAgent : public muse::Agent {
public:
    ClockAgent(muse::AgentID id);

    void initialize() throw (std::exception) override;
    
    void executeTask(const muse::EventContainer& events) override;
    
    void finalize() override;
    
};

#endif

