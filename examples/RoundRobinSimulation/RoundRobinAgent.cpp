#ifndef ROUND_ROBIN_AGENT_CPP
#define	ROUND_ROBIN_AGENT_CPP

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

#include "RoundRobinAgent.h"
#include "Event.h"

RoundRobinAgent::RoundRobinAgent(const muse::AgentID& id, const int max_a) :
    Agent(id, new muse::State()), max_agents(max_a) {
    // Nothing else to be done in the constructor.
}

void
RoundRobinAgent::initialize() {
    if (getAgentID() == 0) {
        const muse::AgentID nextID = (getAgentID() + 1) % max_agents;
	muse::Event* e = muse::Event::create(nextID, getTime() + 1); 
        scheduleEvent(e);
    }
}

void
RoundRobinAgent::executeTask(const muse::EventContainer& events) {
    if (!events.empty()){
        const muse::AgentID nextID = (getAgentID() + 1) % max_agents; 
	muse::Event* e = muse::Event::create(nextID, getTime() + 1); 
        if (getAgentID() == 0)  {
            oss << "Passed token @ time: " << getTime() << std::endl;
        }
        scheduleEvent(e);
    }
} 

void
RoundRobinAgent::finalize() {
    // Nothing to be done here.
} 

#endif 

