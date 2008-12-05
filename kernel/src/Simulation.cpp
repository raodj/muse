#ifndef _MUSE_SIMUALTION_CPP_
#define _MUSE_SIMUALTION_CPP_

#include "Simulation.h"

using namespace muse;

const AgentContainer* Simulation::getRegisteredAgents(){ 
	return allAgents;
}

const SimulatorID Simulation::getSimulatorID(){ return this->_myID; }


bool Simulation::registerAgent(  muse::Agent* agent)  { 
	//add to the agents list
    if (scheduler.addAgentToScheduler(agent->getAgentID())){
        allAgents.insert(agent);
        return true;
    }//end if
    return false;
}//end registerAgent


//TODO: for now the simulator id is default, but needs to get
//fixed to work with MPI
Simulation& 
Simulation::getSimulator(){
    Time startGVT = 0;SimulatorID myID = 1;
    static Simulation kernel(startGVT,myID);
    return kernel;
}


bool 
 Simulation::scheduleEvent( Event *e){
     return scheduler.scheduleEvent(e);
 }

void 
Simulation::start(){
    AgentContainer::iterator it;
    //loop for the initialization
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
        (*it)->initialize();
    }//end for
    
    //BIG loop for event processing 
    EventContainer *events = NULL;
    //while(true){
        for (it=allAgents.begin(); it != allAgents.end(); ++it){
            events = scheduler.getNextEvents((*it)->getAgentID());
            if (events != NULL){
                //this means we have events to process
                (*it)->executeTask(events);
            }//end if
            //(*it)->initialize();
        }//end for
    //}//end BIG loop
    //since we are done with the events pointer time to delete
    delete events;
    
    //loop for the finalization
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
        (*it)->finalize();
    }//end for
}//end start

void Simulation::stop(){}

void 
Simulation::setStartTime(const Time & startTime){
    _startTime = startTime;
}

void 
Simulation::setStopTime(const Time & stopTime){
    _endTime = stopTime;
}

const Time& 
SimulationKernel::getTime(){
    return _LGVT;
}

const Time& 
SimulationKernel::getStartTime(){
    return _startTime;
}

const Time& 
SimulationKernel::getEndTime(){
    return _endTime;
}

#endif