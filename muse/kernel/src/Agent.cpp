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
#include "Simulation.h"
using namespace muse;

void
Agent::initialize() throw (std::exception) {}

void
Agent::executeTask(const EventContainer *events){}

void
Agent::finalize() {}

//-----------------remianing methods are defined by muse-----------
Agent::Agent(AgentID & id, State * agentState) : _myID(id), _LVT(0),_myState(agentState) {}

Agent::~Agent() {}

State*
Agent::cloneState(State & state){
    return state.getClone();
}

void
Agent::setState(State * state){
    //TODO: verify this works!!
    this->_myState = state;
}

void
Agent::updateLVT(const Time & time){
    this->_LVT = time;
}

const 
AgentID& Agent::getAgentID(){
    return _myID;
}

bool 
Agent::scheduleEvent(Event *e){
    if ((Simulation::getSimulator()).scheduleEvent(e)){
        //this means event was scheduled with no problems
        //now lets add this event to our outputQueue in case of rollback
        this->_outputQueue.push_back(e);
        return true;
    }
    return false;
}

//bool
//Agent::scheduleEvents(EventContainer * events){
//    if ((Simulation::getSimulator()).scheduleEvents(events)){
//        //this means event was scheduled with no problems
//        //now lets add this event to our outputQueue in case of rollback
//        list<Event*>::iterator it = _outputQueue.end();
//        this->_outputQueue.insert(it, events->begin(),events->end());
//        return true;
//    }
//    return false;
//}

#endif
