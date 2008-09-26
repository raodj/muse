#ifndef _MUSE_AGENT_H_
#include "../../include/Agent.h"
#endif

using namespace muse;

void Agent::initialize(){}
void Agent::executeTask(const EventContainer &events){}
void Agent::finalize(){}

//-----------------remianing methods are defined by muse-----------
Agent::Agent(){}
const State* Agent::cloneState(const State &state) { return NULL; }
bool Agent::createAgent(const Agent &agent) { return false; }
Stream* Agent::createStreamer(){ return NULL; }
const AgentID* Agent::getAgentID(){ return NULL; }
const Time* Agent::getSimulationTime(TimeType type=NOW){ return NULL;}
bool Agent::migrateAgent(const AgentID &otherAgentID){ return false; }
bool Agent::scheduleEvent(const Event &e){ return false; }
bool Agent::serialize(std::ostream &is){ return false; }
bool Agent::unregisterAgent(){ return false; }
Agent::~Agent(){}