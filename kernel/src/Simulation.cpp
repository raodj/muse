#ifndef _MUSE_SIMUALTION_CPP_
#define _MUSE_SIMUALTION_CPP_

#include "Simulation.h"
#include "Agent.h"

using namespace muse;

Simulation::Simulation(Time & lgvt, SimulatorID & myID): _LGVT(lgvt), _myID(myID), _startTime(0), _endTime(0) {}


const AgentContainer& Simulation::getRegisteredAgents(){
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
    Time startLGVT = 0;SimulatorID myID = 0;
    static Simulation kernel(startLGVT,myID);
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
    while(this->_startTime < this->_endTime){
        for (it=allAgents.begin(); it != allAgents.end(); ++it){
            events = scheduler.getNextEvents((*it)->getAgentID());
            if (events != NULL){
                //this means we have events to process
                //update the agents LVT
                (*it)->updateLVT((*events->begin())->getReceiveTime());
                //execute the agent task
                (*it)->executeTask(events);
                //time to archive the agent's state
                (*it)->stateQueue.push_back(((*it)->_myState).getClone());
            }//end if
        }//end for

        //increase start time by one timestep
        _startTime++;
    }//end BIG loop
    
    //since we are done with the events pointer time to delete
    EventContainer::iterator eventsit;
    for (eventsit=events->begin(); eventsit != events->end(); ++eventsit ){
        delete (*eventsit);
    }//end for
    delete events;
    
    //loop for the finalization
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
        (*it)->finalize();
        //lets take care of all the states still not removed
        list<State*>::iterator state_it;
        for (state_it=(*it)->stateQueue.begin(); state_it != (*it)->stateQueue.end(); ++state_it ){
            delete (*state_it);
        }//end for
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
Simulation::getTime(){
    return _LGVT;
}

const Time& 
Simulation::getStartTime(){
    return _startTime;
}

const Time& 
Simulation::getEndTime(){
    return _endTime;
}

#endif
