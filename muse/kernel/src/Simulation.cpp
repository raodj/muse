#ifndef MUSE_SIMUALTION_CPP
#define MUSE_SIMUALTION_CPP

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
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "Communicator.h"
#include "Simulation.h"
#include "GVTManager.h"
#include "Scheduler.h"

// The number of iterations of event processing after which GVT
// estimation is triggered
#define GVT_DELAY 1

using namespace muse;

Simulation::Simulation() :  LGVT(0), startTime(0), endTime(0) {
    commManager = new Communicator();
    scheduler   = new Scheduler();
    myID = -1u;
}

void
Simulation::initialize(){
    // The following initialization creates dummy arguments to be
    // passed to initialize MPI. Without these parmaeters mpi
    // initialization causes problems.  This is a workaround for a bug
    // seemingly present in MPI.
    int x            = 0;
    char emptyStr[1] = "";  // An empty string
    char **arg       = new char*[1];
    arg[0]           = emptyStr;
    // Initialize MPI and determine the ID of this simulation based on
    // the MPI rank associated with this process.
    myID             = commManager->initialize(x, arg);
    // Free memory as MPI no longer needs this information.
    delete[] arg;
}

void
Simulation::initialize(int argc, char* argv[]){
    myID = commManager->initialize(argc, argv);
}

Simulation::~Simulation() {
    delete scheduler;
    delete commManager;
}

const AgentContainer&
Simulation::getRegisteredAgents(){
    return allAgents;
}

bool
Simulation::registerAgent(  muse::Agent* agent)  { 
    if (scheduler->addAgentToScheduler(agent)){
        allAgents.push_back(agent);
        return true;
    }
    return false;
}//end registerAgent


Simulation*
Simulation::getSimulator(){
    static Simulation kernel;
    return &kernel; // address of sole instance
}//end getSimulator


bool 
Simulation::scheduleEvent( Event *e){
    AgentID recvAgentID = e->getReceiverAgentID();
    if (isAgentLocal(recvAgentID)){
        // Local events are directly inserted into our own scheduler
        return scheduler->scheduleEvent(e);
    }
    else{
        // Remote events are sent via the GVTManager to aid tracking
        // GVT. The gvt manager calls communicator.
        gvtManager->sendRemoteEvent(e);
    }
    return true;
}


void 
Simulation::start(){
    //first we setup the AgentMap for all kernels
    commManager->registerAgents(allAgents);
    // Create and initialize our GVT manager.
    gvtManager = new GVTManager();
    gvtManager->initialize(startTime, commManager);
    // Set gvt manager with the communicator.
    commManager->setGVTManager(gvtManager);
    
    //loop for the initialization
    AgentContainer::iterator it;
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
         (*it)->initialize();
        //time to archive the agent's init state
         State *agent_state = (*it)->myState;
         State * state = (*it)->cloneState( agent_state );
         //cout << "agent :"<<(*it)->getAgentID()<< " first state timestamp: "<<state->getTimeStamp()<<endl;
         (*it)->stateQueue.push_back(state);
        
    }//end for
    

    LGVT = startTime;
   
    //BIG loop for event processing
    //int count=0;
    int gvtTimer = GVT_DELAY;
   
    while(gvtManager->getGVT() < endTime){
        if (myID == 0 ) cout << "GVT @ time: " << gvtManager->getGVT() << endl;
        
        if (--gvtTimer == 0 ) {
            gvtTimer = GVT_DELAY;
            //cout << "[Simulation] starting startGVTestimation*********" <<endl;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        Event* incoming_event = commManager->receiveEvent();
        if ( incoming_event != NULL ){	  
            scheduleEvent(incoming_event);
        } //end if

        //process the next agent
        scheduler->processNextAgentEvents();
    }//end BIG loop
    if (myID == 0 ) cout << "GVT @ end: " << gvtManager->getGVT() << endl;
    
}//end start

void
Simulation::finalize(){
    myID = -1u;
    //loop for the finalization
    AgentContainer::iterator it=allAgents.begin();
    for (; it != allAgents.end(); it++) {  
        (*it)->finalize();
        (*it)->cleanInputQueue();
        (*it)->cleanStateQueue();
        (*it)->cleanOutputQueue();
        delete (*it);  
    }//end for

    // Now delete GVT manager as we no longer need it.
    commManager->setGVTManager(NULL);
    delete gvtManager;
    gvtManager = NULL;
    
    //finalize the communicator
    commManager->finalize();
}//end finalize

void
Simulation::garbageCollect(const Time gvt){
    AgentContainer::iterator it=allAgents.begin();
     for (; it != allAgents.end(); it++) {  
         (*it)->garbageCollect(gvt);
     }
}


bool
Simulation::isAgentLocal(AgentID & id){ return commManager->isAgentLocal(id); }

void Simulation::stop(){}

void 
Simulation::setStartTime(Time start_time){
    startTime = start_time;
}

void 
Simulation::setStopTime( Time  end_time){
    endTime = end_time;
}

Time
Simulation::getLGVT() const {
    return scheduler->getNextEventTime();
}

Time
Simulation::getTime() const {
    return gvtManager->getGVT();
}

void
Simulation::changeKey(void* pointer, Agent * agent){
    scheduler->changeKey(pointer,agent);
}

#endif
