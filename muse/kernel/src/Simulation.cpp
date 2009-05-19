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

Simulation::Simulation() :  LGVT(0), startTime(0), endTime(0),gvt_delay_rate(10), number_of_processes(-1u) {
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

    //here we hijack cerr any move the data to file for logging.
    DEBUG({
        char logFileName[128];
        sprintf(logFileName, "Log%u.txt", myID);
        logFile = new  ofstream(logFileName);
        oldstream = std::cout.rdbuf(logFile->rdbuf());
    });
}

Simulation::~Simulation() {
    delete scheduler;
    delete commManager;
    DEBUG(delete logFile);
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
        cerr << "Dont use this method with a new event, go "
             << "through the agent's scheduleEvent method." <<endl;
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

    //information about the simulation environment
    unsigned int rank         = -1u;
    commManager->getProcessInfo(rank, number_of_processes);
    
    //loop for the initialization
    AgentContainer::iterator it;
    for (it=allAgents.begin(); it != allAgents.end(); ++it){
         (*it)->initialize();
         // time to archive the agent's init state
         State *agent_state = (*it)->getState();
         State * state = (*it)->cloneState( agent_state );
        
         if (getNumberOfProcesses() > 1 ){
             (*it)->stateQueue.push_back(state);
         }
        
    }//end for
    
    LGVT = startTime;
   
    //BIG loop for event processing
    //int count=0;
    int gvtTimer              = gvt_delay_rate;
    

    while(gvtManager->getGVT() < endTime){
        if (--gvtTimer == 0 ) {
            gvtTimer = gvt_delay_rate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }

        //if we only have one process, then we don't need to check the wire
        if ( getNumberOfProcesses() > 1 ) {
            //A optimization trick we learned from WARPED is to try to
            //get as many event from the wire as we can. A good magic
            //number is 1000
            for (int magic=0; magic < 1000 ; magic++ ){
                Event* incoming_event = commManager->receiveEvent();
                if ( incoming_event != NULL ){	  
                    scheduleEvent(incoming_event);
                    if ((LGVT = scheduler->getNextEventTime()) < getGVT()) {
                        std::cout << "LGVT = " << LGVT << " is below GVT: "
                                  << getGVT()
                                  << " which is serious error. "
                                  << "Scheduled agents:\n";
                        scheduler->agent_pq.prettyPrint(std::cout);
                        std::cerr << "Rank " << myID << " Aborting.\n";
                        std::cerr << std::flush;
                        DEBUG(logFile->close());
                        ASSERT ( false );
                    }
                }else{
                    break; 
                }
            }//end magic mpi for loop
        }//end check if
        
        // Update lgvt to the time of the next event to be processed.
        LGVT = scheduler->getNextEventTime();
        if (LGVT < getGVT()) {
            std::cout << "LGVT = " << LGVT << " is below GVT: " << getGVT()
                      << " which is serious error. Scheduled agents: \n";
            scheduler->agent_pq.prettyPrint(std::cerr);
            std::cerr << "Rank " << myID << " Aborting.\n";
            std::cerr << std::flush;
            DEBUG(logFile->close());
            ASSERT ( false );
        }
        
        scheduler->processNextAgentEvents();
        
    }//end BIG loop
    
    
}//end start

void
Simulation::finalize(){
    //loop for the finalization
    AgentContainer::iterator it=allAgents.begin();
    int total_rollbacks =0, total_committed_events=0, total_mpi_messages=0, total_scheduled_events=0;
    for (; it != allAgents.end(); it++) {  
        (*it)->finalize();
        (*it)->cleanInputQueue();
        (*it)->cleanStateQueue();
        (*it)->cleanOutputQueue();
        
        total_rollbacks += (*it)->num_rollbacks;
        total_committed_events += (*it)->num_processed_events;
        total_mpi_messages += (*it)->num_mpi_messages;
        total_scheduled_events += (*it)->num_mpi_messages;
        
        
        delete (*it);  
    }//end for
    std::cout << "\nKernel[" << myID
                  <<"]\n Total Scheduled Events[" <<total_scheduled_events 
                  <<"]\n Total Committed Events[" <<total_committed_events
                  << "]\n Total rollbacks[" << total_rollbacks
                  << "]\n Total MPI messages[" << total_mpi_messages << "]"<<std::endl;
    //std::cout << "Done with agents: SHUTTING DOWN" <<endl;
    // Now delete GVT manager as we no longer need it.
    commManager->setGVTManager(NULL);
    delete gvtManager;
    gvtManager = NULL;
    
    //finalize the communicator
    commManager->finalize();

    //invalid kernel id
    myID = -1u;
    
    //lets give cerr back its streambuf
    DEBUG(std::cerr.rdbuf(oldstream));
}//end finalize

void
Simulation::garbageCollect(){
    
    AgentContainer::iterator it=allAgents.begin();
    for (; it != allAgents.end(); it++) {  
        (*it)->garbageCollect(getGVT());
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
Simulation::updateKey(void* pointer,  Time old_top_time){
    scheduler->updateKey(pointer,old_top_time);
}

#endif
