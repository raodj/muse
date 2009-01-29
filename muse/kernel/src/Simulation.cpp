
#ifndef _MUSE_SIMUALTION_CPP_
#define _MUSE_SIMUALTION_CPP_
#include "Communicator.h"
#include "Simulation.h"

using namespace muse;

Simulation::Simulation() :  LGVT(0), startTime(0), endTime(0) {
    myID = -1u;
}

void
Simulation::initialize(){
    int x=0;
    char **arg = new char*[1];
    arg[0] = "";
    myID = commManager.initialize(x,arg);
    delete[] arg;
}

void
Simulation::initialize(int argc, char* argv[]){
    myID = commManager.initialize(argc,argv);
}


Simulation::~Simulation() {}

const AgentContainer& Simulation::getRegisteredAgents(){
	return allAgents;
}

SimulatorID Simulation::getSimulatorID(){ return this->myID; }


bool Simulation::registerAgent(  muse::Agent* agent)  { 
	//add to the agents list
    if (scheduler.addAgentToScheduler(agent)){
        allAgents.push_back(agent);
        return true;
    }//end if
    return false;
}//end registerAgent

//Simulation kernel; // initialize kernel pointer

Simulation*
Simulation::getSimulator(){
    static Simulation kernel;
    return &kernel; // address of sole instance
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
        //time to archive the agent's init state
        State * state = (*it)->myState->getClone();
        (*it)->stateQueue.push_back(state);
    }//end for

    LGVT = startTime;

    //BIG loop for event processing 
    while(LGVT < endTime){
        //if (myID == 0 ) cout << "Root Ticker @ time: " <<LGVT<< endl;
        //here we poll the wire for any new events to add
        //NOTE:: we should also look into detecting rollbacks here!!!
        Event* incoming_event = commManager.receiveEvent();
        if ( incoming_event != NULL ){
            scheduleEvent(incoming_event);
        } //end if

         //use this to calculate the min LVT
        Time min_lvt = 1e30 ;
        //loop through all agents and process their events
        for (it=allAgents.begin(); it != allAgents.end(); ++it){
            if (scheduler.processNextAgentEvents() ){
                //check for the smallest LVT here!!
                if ((*it)->getLVT() < min_lvt) {
                    min_lvt = (*it)->getLVT();
                }//end if
            }//end if
        }//end for
       
        //increase start time by one timestep
        if (min_lvt < 1e30) {
            LGVT = min_lvt;
        }//end if

    }//end BIG loop
}//end start

void
Simulation::finalize(){
    myID = -1u;
    //loop for the finalization
    AgentContainer::iterator it;
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
        (*it)->finalize();

        //lets take care of all the events in the inputQueue aka processed Events
        list<Event*>::iterator event_it;
        //cout << "[Simulation] - killing input" << endl;
//        for (event_it=(*it)->inputQueue.begin(); event_it != (*it)->inputQueue.end(); ++event_it ){
//            if ((*event_it) != NULL){
//            //cout << "[Simulation] - input event ref count: " <<(*event_it)<< " - " << (*event_it)->referenceCount<< endl;
//                (*event_it)->deleteEvent();
//                (*event_it)=NULL;
//            }
//           // cout << "[Simulation] - input event ref countW: " <<(*event_it)<< " - " << (*event_it)->referenceCount<< endl;
//
//        }//end for

        //lets take care of all the states still not removed
        list<State*>::iterator state_it;
        for (state_it=(*it)->stateQueue.begin(); state_it != (*it)->stateQueue.end(); ++state_it ){
            delete (*state_it);
        }//end for

       //now lets delete all remaining events in each agent's outputQueue
//        list<Event*>::iterator eit;
//        //cout << "[Simulation] - killing output" << endl;
//        for (eit=(*it)->outputQueue.begin(); eit != (*it)->outputQueue.end(); ++eit ){
//            //cout << "[SIMULATION] - output address event ref count: "<< (*eit)<< " - "<<(*eit)->referenceCount << endl;
//            if ((*eit) != NULL) {
//                (*eit)->deleteEvent();
//                (*eit) = NULL;
//            }
//            //cout << "[SIMULATION] - output address event ref countW: "<< (*eit)<< " - "<<(*eit)->referenceCount << endl;
//
//        }//end for

        //(*it)->inputQueue.clear();
        //(*it)->stateQueue.clear();
        //(*it)->outputQueue.clear();
    }//end for

    //finalize the communicator
    commManager.finalize();
}//end finalize


void Simulation::stop(){}

void 
Simulation::setStartTime(const Time & start_time){
    startTime = start_time;
}

void 
Simulation::setStopTime(const Time & end_time){
    endTime = end_time;
}

const Time& 
Simulation::getTime(){
    return LGVT;
}

const Time& 
Simulation::getStartTime(){
    return startTime;
}

const Time& 
Simulation::getEndTime(){
    return endTime;
}

#endif
