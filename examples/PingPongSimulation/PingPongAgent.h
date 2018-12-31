/* 
 * File:   PingPongAgent.h
 * Author: gebremr
 *
 * Created on January 15, 2009, 11:04 PM
 */

#ifndef PING_PONG_AGENT_H
#define	PING_PONG_AGENT_H

#include "Agent.h"
#include "State.h"

class PingPongAgent : public muse::Agent {
public:
    /**
     * Constructor to create a ping pong agent.
     * 
     * @param agentID The unique ID to assign to this agent. This
     * value is set once when an agent is instantiated and is never
     * changed during the life time of this agent. Note that a default
     * (but invalid) value of -1 is set to ease creation of an vector
     * of agents.
     */
    PingPongAgent(const muse::AgentID &agentID = -1);

    /**
     * The initialize method for this object. The initialize method
     * is called only once when an object is created. This method 
     * essentially generates the first event in the simulation 
     * only on agent with ID 0.
     */
    void initialize() override;

    /**
     * This method is called each time this agent has an event
     * to process. This method essentially creates a new event
     * and dispatches it to a different agent in the simulation.
     * 
     * @param events The list of events to process. This list 
     * will contain at least one event for this agent to process.
     */
    void executeTask(const muse::EventContainer& events) override;

    /**
     * This method is called once just before the simulation 
     * completes. This method currently just prints a simple
     * message.
     */
    void finalize() override;

};

#endif	/* _PINGPONGAGENT_H */

