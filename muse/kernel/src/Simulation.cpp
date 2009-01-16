
#ifndef _MUSE_SIMUALTION_CPP_
#define _MUSE_SIMUALTION_CPP_
#include "Communicator.h"
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

SimulatorID Simulation::getSimulatorID(){ return this->_myID; }


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
    AgentID recvAgentID = e->getReceiverAgentID();
     if (commManager.isAgentLocal(recvAgentID)){
       // cout << "[SIMULATION] Agent is local: sending event directly to scheduler!" << endl;
        return scheduler.scheduleEvent(e);
     }
     else{
         //cout << "[SIMULATION] Agent is NOT local: sending event via CommManager!" << endl;
         int size = sizeof(*e);
         commManager.sendEvent(e,size);
     }
     //cout << "[SIMULATION] - made it in scheduleEvent" << endl;
     return true;
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
    while(this->_startTime < this->_endTime){
        cout << "Ticker @ time: " <<_startTime<< endl;
        //here we poll the wire for any new events to add
        //NOTE:: we should also look into detecting rollbacks here!!!
        Event* incoming_event = commManager.receiveEvent();
        if ( incoming_event != NULL ){
            //cout << "Got an Event from CommManager!!" << endl;
            scheduleEvent(incoming_event);
        }
        
        //loop through all agents and process their events
        for (it=allAgents.begin(); it != allAgents.end(); ++it){
            EventContainer *events = scheduler.getNextEvents((*it)->getAgentID());
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
                //now we must store the events in the agent's inputQueue
                list<Event*>::iterator event_it;
                for (event_it=(*it)->_inputQueue.begin(); event_it != (*it)->_inputQueue.end(); ++event_it ){
                    (*it)->_inputQueue.push_back((*event_it));
                }//end for
            }//end if
        }//end for

        //increase start time by one timestep
        _startTime++;
    }//end BIG loop
    
    //loop for the finalization
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
        (*it)->finalize();
        //cout << "[SIMULATION] - called agent finalize" << endl;
        
        //lets take care of all the events in the inputQueue aka processed Events
        list<Event*>::iterator event_it;
        for (event_it=(*it)->_inputQueue.begin(); event_it != (*it)->_inputQueue.end(); ++event_it ){
            delete (*event_it);
        }//end for
        //cout << "[SIMULATION] - deleted remaining events in agent's inQ" << endl;

        //lets take care of all the states still not removed
        list<State*>::iterator state_it;
        for (state_it=(*it)->_stateQueue.begin(); state_it != (*it)->_stateQueue.end(); ++state_it ){
            delete (*state_it);
        }//end for
        //cout << "[SIMULATION] - deleted all states in agent's stateQ" << endl;

        //now lets delete all remaining events in each agent's outputQueue
        list<Event*>::iterator eit;
        for (eit=(*it)->_outputQueue.begin(); eit != (*it)->_outputQueue.end(); ++eit ){
            delete (*eit);
        }//end for
        //cout << "[SIMULATION] - deleted remaining events in agent's outQ" << endl;
    }//end for

    //cout << "[SIMULATION] - almost done with cleanup!" << endl;
    //finalize the communicator
    commManager.finalize();
    //cout << "[SIMULATION] - commManager should be finalized!!" << endl;
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
