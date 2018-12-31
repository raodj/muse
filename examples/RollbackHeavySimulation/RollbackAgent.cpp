#ifndef ROLLBACK_AGENT_CPP
#define	ROLLBACK_AGENT_CPP


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

#include "RollbackAgent.h"
#include "Event.h"
#include <iostream>

RollbackAgent::RollbackAgent(muse::AgentID& id, const int max_a) :
    Agent(id, new muse::State()), max_agents(max_a) {}

void
RollbackAgent::initialize() {
    if (getAgentID() == 0) {
	muse::Time receive = 1;
	muse::Event* const e = muse::Event::create(getAgentID(), receive); 
	scheduleEvent(e);
    }
} //end initialize

void
RollbackAgent::executeTask(const muse::EventContainer& events) {
    if (!events.empty()) {
        const muse::Time receiveTime = getTime() + 1;
        for (int i = 0; (i < max_agents); i++) {
	    muse::Event* const e = muse::Event::create(muse::AgentID(i),
						       receiveTime);
	    scheduleEvent(e);
	}
    }
} //end executeTask

void
RollbackAgent::finalize() {
    std::cout << "RollbackAgent [" << getAgentID()
	      << "] finalized."    << std:: endl;
} //end finalize

#endif 

