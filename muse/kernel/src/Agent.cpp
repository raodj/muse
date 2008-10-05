#ifndef MUSE_AGENT_CPP
#define MUSE_AGENT_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Meseret Gebre       gebremr@muohio.edu
//          Dhananjai M. Rao    raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "Agent.h"

using namespace muse;

void
Agent::initialize() throw (std::exception) {}

void
Agent::executeTask(const EventContainer &events){}

void
Agent::finalize() throw () {}

//-----------------remianing methods are defined by muse-----------
Agent::Agent(){}

Agent::~Agent() {}

const State*
Agent::cloneState(const State &state) const {
    return NULL;
}

bool
Agent::createAgent(const Agent &agent) {
    return false;
}

Stream*
Agent::createStream(const std::string& name,
                    const std::ios_base::openmode mode) {
    return NULL;
}

const AgentID&
Agent::getAgentID() const {
    return AgentID(-1);
}

const Time*
Agent::getSimulationTime(TimeType type) const {
    return NULL;
}

bool
Agent::migrateAgent(const AgentID &otherAgentID) {
    return false;
}

bool
Agent::scheduleEvent(const Event &e) {
    return false;
}

bool
Agent::serialize(std::ostream &is) const{
    return false;
}

bool
Agent::unregisterAgent() {
    return false;
}

#endif
