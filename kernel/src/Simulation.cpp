
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
    if (isAgentLocal(recvAgentID)){ 
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
        State * state = (*it)->myState;
        (*it)->stateQueue.push_back(state);
    }//end for

    LGVT = startTime;
    //use this to calculate the min LVT
    Time min_lvt = 1e30 ;
    //BIG loop for event processing
    int count=0;
    while(LGVT < endTime){
        //if (myID == 0 ) cout << "Root Ticker @ time: " <<LGVT<< endl;
        //here we poll the wire for any new events to add
        //NOTE:: we should also look into detecting rollbacks here!!!
        Event* incoming_event = commManager.receiveEvent();
        if ( incoming_event != NULL ){
	  
	  scheduleEvent(incoming_event);
        } //end if

        
        //loop through all agents and process their events
        for (it=allAgents.begin(); it != allAgents.end(); it++){
            if (scheduler.processNextAgentEvents() ){
                //check for the smallest LVT here!!
                //cout << "\ngetLVT: " << (*it)->getLVT() <<endl;
                //cout << "min_lvt: " << min_lvt <<endl;
                if ((*it)->getLVT() < min_lvt) {
                    min_lvt = (*it)->getLVT();
                }//end if
            }//end if
        }//end for
       
        //increase start time by one timestep
        if (min_lvt < 1e30) {
            LGVT = min_lvt;
            min_lvt = 1e30 ;
        }//end if
    }//end BIG loop
}//end start

void
Simulation::finalize(){
    myID = -1u;
    //loop for the finalization
    AgentContainer::iterator it=allAgents.begin();
    cout <<" Agents size: " << allAgents.size()<<endl;
    while ( it != allAgents.end()) {
      AgentContainer::iterator del_it = it;
      it++;
      cout << "Finalizing Agent: " << (*del_it)->getAgentID()<<endl;
      (*del_it)->finalize();
      (*del_it)->cleanInputQueue();
      (*del_it)->cleanStateQueue();
      (*del_it)->cleanOutputQueue();
      delete (*del_it);
      //allAgents.erase(del_it);
    }//end for
    
    //finalize the communicator
    commManager.finalize();
}//end finalize

bool
Simulation::isAgentLocal(AgentID & id){ return commManager.isAgentLocal(id); }

void Simulation::stop(){}

void 
Simulation::setStartTime(Time start_time){
    startTime = start_time;
}

void 
Simulation::setStopTime( Time  end_time){
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
