#ifndef _MUSE_SIMULATION_H_
#include "../../include/Simulation.h"
#endif
using muse::Simulation;

Simulation::Simulation(){}

const muse::AgentContainer* Simulation::getRegisteredAgents(){ return NULL;}
const muse::SimulatorID Simulation::getSimulatorID(){ return this->myID; }
const muse::AgentID* Simulation::registerAgent( const muse::Agent* agent) const { return NULL;}

muse::Simulation * Simulation::getSimulator(){ return NULL; }

void Simulation::start(){}
void Simulation::stop(){}

Simulation::~Simulation(){}