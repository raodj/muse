
#include "Communicator.h"

#ifndef _MUSE_SIMUALTION_CPP_
#define _MUSE_SIMUALTION_CPP_

#include "Simulation.h"


using namespace muse;

Simulation::Simulation() :  _LGVT(0), _startTime(0), _endTime(0) {
    _myID = commManager.initialize();
}

Simulation::~Simulation() {
    delete kernel; //make sure we dispose of Simulation instance
}

const AgentContainer& Simulation::getRegisteredAgents(){
	return allAgents;
}

const SimulatorID Simulation::getSimulatorID(){ return this->_myID; }


bool Simulation::registerAgent(  muse::Agent* agent)  { 
	//add to the agents list
    if (scheduler.addAgentToScheduler(agent->getAgentID())){
        allAgents.push_back(agent);
        return true;
    }//end if
    return false;
}//end registerAgent


//TODO: for now the simulator id is default, but needs to get
//fixed to work with MPI
 Simulation* Simulation::kernel = NULL;// initialize kernel pointer

Simulation*
Simulation::getSimulator(){
    if (kernel == NULL)  // is it the first call?
    {
      kernel = new Simulation(); // create sole instance
    }
    return kernel; // address of sole instance
}//end getSimulator


bool 
 Simulation::scheduleEvent( Event *e){
     return scheduler.scheduleEvent(e);
 }

//bool
// Simulatison::scheduleEvents( EventContainer *events){
//     return scheduler.scheduleEvents(events);
// }

void 
Simulation::start(){
    //first we setup the AgentMap for all kernels
    commManager.registerAgents(allAgents);

    //loop for the initialization
    AgentContainer::iterator it;
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
                //might consider creating a method (addToStateQueue(State*))
                ////incase we change _stateQueue implementation as another layer of protection.
                (*it)->_stateQueue.push_back((*it)->_myState->getClone());
            }//end if
        }//end for

        //increase start time by one timestep
        _startTime++;
    }//end BIG loop
    
    //since we are done with the events pointer time to delete
//    EventContainer::iterator eventsit;
//    for (eventsit=events->begin(); eventsit != events->end(); ++eventsit ){
//        delete (*eventsit);
//    }//end for
//    delete events;
    
    //loop for the finalization
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
        (*it)->finalize();
        //lets take care of all the states still not removed
        list<State*>::iterator state_it;
        for (state_it=(*it)->_stateQueue.begin(); state_it != (*it)->_stateQueue.end(); ++state_it ){
            delete (*state_it);
        }//end for

        //now lets delete all remaining events in each agent's outputQueue
        list<Event*>::iterator eit;
        for (eit=(*it)->_outputQueue.begin(); eit != (*it)->_outputQueue.end(); ++eit ){
            delete (*eit);
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
