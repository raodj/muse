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
#include <cstdlib>


using namespace muse;

Simulation::Simulation() :  LGVT(0), startTime(0), endTime(0),gvt_delay_rate(10), one_below_gvt(0) {
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
    ASSERT ( e->getReceiveTime() >= getGVT() );
    if (TIME_EQUALS(e->getSentTime(),TIME_INFINITY) ||
        (e->getSenderAgentID() == -1)) {
        cerr << "Dont use this method with a new event, go through the agent's scheduleEvent method." <<endl;
        abort();
    }
    AgentID recvAgentID = e->getReceiverAgentID();
   
    if (isAgentLocal(recvAgentID)){
        // Local events are directly inserted into our own scheduler
        return scheduler->scheduleEvent(e);
    }else{
        // Remote events are sent via the GVTManager to aid tracking
        // GVT. The gvt manager calls communicator.
        gvtManager->sendRemoteEvent(e);
    }
    return true;
}


void 
Simulation::start(){
    //if no agents registered we need to leave start and end sim
    if (allAgents.empty()) return;
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
         State *agent_state = (*it)->getState();
         State * state = (*it)->cloneState( agent_state );
         //cout << "agent :"<<(*it)->getAgentID()<< " first state timestamp: "<<state->getTimeStamp()<<endl;
         (*it)->stateQueue.push_back(state);
        
    }//end for
    

    LGVT = startTime;
   
    //BIG loop for event processing
    //int count=0;
    int gvtTimer = gvt_delay_rate;

    // this is used to help calculate one_below_gvt
    Time old_lgvt = LGVT;
    
    while(gvtManager->getGVT() < endTime){
        //if (myID == 0 ) cout << "GVT @ time: " << gvtManager->getGVT() << endl;
        
        if (--gvtTimer == 0 ) {
            gvtTimer = gvt_delay_rate;
            //cout << "[Simulation] starting startGVTestimation*********" <<endl;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        Event* incoming_event = commManager->receiveEvent();
        if ( incoming_event != NULL ){	  
            scheduleEvent(incoming_event);
        } //end if

      
      
        // Update lgvt to the time of the next event to be processed.
        LGVT = scheduler->getNextEventTime();
        if (LGVT < getGVT()) {
            std::cerr << "LGVT = " << LGVT << " is below GVT: " << getGVT()
                      << " which is serious error. Scheduled agents: \n";
            scheduler->agent_pq.prettyPrint(std::cerr);
            std::cerr << "Aborting.\n";
            ASSERT ( false );
        }
        
        //lets check if we need to update one_below_gvt value
        //cerr << "OLD_LGVT: " << old_lgvt << " LGVT: " << LGVT
        //     << " ONE_BELOW_GVT: " << one_below_gvt << " GVT: " << getGVT()<<endl;
        if ( old_lgvt < getGVT() ){
            one_below_gvt = old_lgvt;
            old_lgvt = LGVT;
        }
        scheduler->processNextAgentEvents();
        //if (!was_event_processed) cout << "[Simulation] no events to process at this time..." << endl;
    }//end BIG loop
    
    
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
Simulation::garbageCollect(){
    
    AgentContainer::iterator it=allAgents.begin();
    for (; it != allAgents.end(); it++) {  
        (*it)->garbageCollect(one_below_gvt);
    }//end for
    
}//end garbageCollect


bool
Simulation::isAgentLocal(AgentID id){ return commManager->isAgentLocal(id); }

void Simulation::stop(){}


Time
Simulation::getLGVT() const {
    return LGVT;
}

Time
Simulation::getGVT() const {
    return gvtManager->getGVT();
}

void
Simulation::changeKey(void* pointer, Agent * agent){
    scheduler->changeKey(pointer,agent);
}

#endif
