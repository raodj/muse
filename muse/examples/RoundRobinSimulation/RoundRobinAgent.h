#ifndef ROUND_ROBIN_AGENT_H
#define	ROUND_ROBIN_AGENT_H

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "Agent.h"

/* Agent that fowards a token (aka event) to adjacent agent.

   This class provides an implementation for straightforward agent
   that forwards a token (single event) to its adjacent neighboring
   agent.  Agents are assumed to be organized in a logical circle.
   Agent number 0 (zero) initiates the token/event that is circulated
   in a cycle.  A the end of each cycle, agent 0 prints a message
   indicating the token has completed another round.
*/
class RoundRobinAgent : public muse::Agent {
public:
    /** The constructor.

	The constructor creates an initial default state for the agent
	and initializes other instance variables in the class.
    */
    RoundRobinAgent(const muse::AgentID& id, const int max_agents);

    /** The initialization API method that overrides default in base
	class.

	The initialize method is invoked by the MUSE kernel just when
	the simulation commences.  This method in agent number zero
	generates the initial token/event that is circulated to
	adjacent agents.
    */
    void initialize() throw (std::exception);

    /** The method to process event(s) scheduled for this agent.

	This method is invoked whenever this agent has an event to
	process.  This agent logically forwards the token to its
	adjacent agent to be recieved at the next simulation time.
	Adjacent neighbors are detected using the formula (id + 1) %
	max_agents.

	\param[in] events The list of events to be processed by this
	agent. Currently, the agent assumes only one event is ever
	scheduled and does not really use the events in any meaningful
	manner.
    */
    void executeTask(const muse::EventContainer* events);

    /** API method invoked by the MUSE kernel at the end of simulation.

	Currently, this method does not have any special tasks to
	perform and is present merely as a place holder.
    */
    void finalize();

private:
    /**
       The total number of agents in the simulation.  This value is
       used to logically place agents in a circle so that the last
       agent sends the event/token to agent number 0 (zero).
    */
    const int max_agents;
};

#endif

