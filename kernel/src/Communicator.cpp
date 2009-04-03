#ifndef MUSE_COMMUNICATOR_CPP
#define MUSE_COMMUNICATOR_CPP
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
// Authors:  Meseret R. Gebre       meseret.gebre@gmail.com
//           
//
//---------------------------------------------------------------------------

#include "Communicator.h"
#include "GVTManager.h"
#include "GVTMessage.h"
#include "Event.h"
#include "Agent.h"
#include <mpi.h>
#include "DataTypes.h"

using namespace muse;

Communicator::Communicator(){
    gvtManager = NULL;
}

SimulatorID
Communicator::initialize(int argc, char* argv[]){
    MPI::Init(argc,argv); 
    return MPI::COMM_WORLD.Get_rank();
}

void
Communicator::registerAgents(AgentContainer& allAgents){
    int  size = MPI::COMM_WORLD.Get_size();

    //first lets add all kernel add local agents to map!
    AgentContainer::iterator ac_it; //ac == AgentContainer
    for (ac_it=allAgents.begin(); ac_it != allAgents.end(); ++ac_it){
         agentMap[(*ac_it)->getAgentID()] = ROOT_KERNEL;
    }//end for

    //if size == 1 : then we need not do this!
    if (size == 1) {
        //cout << "[Communicator] Only kernel around: Registration is avoided" << endl;
        return;
    }
    int  simulator_id = MPI::COMM_WORLD.Get_rank();

    if (simulator_id == ROOT_KERNEL){
        //here we collect all agent id from all other simulation kernels!!
        int done_count = 1;
        MPI::Status status;
        while ( !(done_count == size)){
            //probe for a message from another simulation kernel.
            MPI::COMM_WORLD.Probe(MPI_ANY_SOURCE, AGENT_LIST, status);
            //figure out the agent list size
            int agent_list_size = status.Get_count(MPI::UNSIGNED);
            unsigned int agentList[agent_list_size];
            //now receive the agent id flat list
            MPI::COMM_WORLD.Recv( &agentList ,agent_list_size, MPI::UNSIGNED , MPI_ANY_SOURCE , AGENT_LIST, status );
            //time to add this list to master AgentMap
            for(int i=0; i<agent_list_size; ++i){
                agentMap[agentList[i]] = status.Get_source();
            }//end for 
            done_count++;
        }//end while
        
        //next chunk of code converts agentMap to a flat list for Bcasting!!
        AgentIDSimulatorIDMap::iterator it;
        int agentMap_size = agentMap.size()*2;
        unsigned int  agentMap_flat[agentMap_size];
        int counter=0;
        for (it=agentMap.begin(); it != agentMap.end(); ++it){
            ///cout << "Agent ID: " << it->first << " Registered to Kernel ID: " << it->second << endl;
            agentMap_flat[counter] = it->first;
            agentMap_flat[counter+1] = it->second;
            counter+=2;
        }//end for
        
        //before we bcast the agentMap, we must Bcast the size of the list!
        MPI::COMM_WORLD.Bcast(&agentMap_size, 1 ,MPI::INT, ROOT_KERNEL );
        //bcast here
        MPI::COMM_WORLD.Bcast(&agentMap_flat, agentMap_size,MPI::UNSIGNED, ROOT_KERNEL );
        
        
    }else{
        AgentID agentList[allAgents.size()];
        for (size_t i=0; i < allAgents.size(); ++i){
            agentList[i] = allAgents[i]->getAgentID();//populate the flat list.
        }//end for
        //now send flat list across the wire (MPI)
        MPI::COMM_WORLD.Send(&agentList, allAgents.size(), MPI::UNSIGNED,ROOT_KERNEL,AGENT_LIST);
        //get the size of incoming agentMap list
        int agentMap_length;
        MPI::COMM_WORLD.Bcast(&agentMap_length, 1 ,MPI::INT, ROOT_KERNEL );
        
        //finally receive the complete agentMap flat list!!!
        unsigned int agentMap_flatList[agentMap_length];
        MPI::COMM_WORLD.Bcast(&agentMap_flatList, agentMap_length,MPI::UNSIGNED, ROOT_KERNEL );
        
        //use final flat list and populate local AgentMap list!!
        for (int i=0; i < agentMap_length; i+=2){
            agentMap[agentMap_flatList[i]] = agentMap_flatList[i+1];
            //cout << "Agent ID: " << agentMap_flatList[i] << " Registered to Kernel ID: " << agentMap_flatList[i+1] << endl;
        }//end for
        
    }//end if
}//end registerAgent method

void
Communicator::sendEvent(Event * e, const int event_size){
    
    try {
        //no good way to send objects via MPI make Event to a char*
        //aka ghetto hack :-)
        char* serialEvent = reinterpret_cast<char*>(e);
        MPI::COMM_WORLD.Send(serialEvent, event_size, MPI::CHAR,
                             agentMap[e->getReceiverAgentID()],EVENT);
        //cout << "SENT an Event of size: " << event_size << endl;
        //cout << "[COMMUNICATOR] - made it in sendEvent" << endl;
        //e->decreaseReference();
    } catch (MPI::Exception e) {
        cerr << "MPI ERROR (sendEvent): ";cerr << e.Get_error_string() << endl;
    }
}//end sendEvent

void
Communicator::sendMessage(const GVTMessage *msg, const int destRank) {
    try {
        // GVT messages are already serialized.
        const char *data = reinterpret_cast<const char*>(msg);
        MPI::COMM_WORLD.Send(data, msg->getSize(), MPI::CHAR,
                             destRank, GVT_MESSAGE);
    } catch (MPI::Exception e) {
        std::cerr << "MPI ERROR (sendMessage): ";
        std::cerr << e.Get_error_string() << std::endl;
    }
}



Event*
Communicator::receiveEvent(){
    MPI::Status status;
    try {
        if (!MPI::COMM_WORLD.Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, status)) {
            // No pending event.
            return NULL;
        }
    } catch (MPI::Exception e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        return NULL;
    }
    //figure out the agent list size
    int event_size = status.Get_count(MPI::CHAR);
    char *incoming_event = new char[event_size];
    // Read the actual data.
    try {
        MPI::COMM_WORLD.Recv(incoming_event, event_size, MPI::CHAR,
                             status.Get_source(), status.Get_tag(), status);
    } catch (MPI::Exception e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        delete[] incoming_event;
        return NULL;
    }
    
    // Now handle the incoming data based on the tag value.
    if (status.Get_tag() == EVENT) {
        // Type cast does the trick as events are binary blobs
        Event* the_event = reinterpret_cast<Event*>(incoming_event);
        // Ensure some of the core data is valid.
        
        ASSERT( gvtManager != NULL );
        // Let GVT manager inspect incoming events.
        gvtManager->inspectRemoteEvent(the_event);
        // Dispatch event for further processing.
        return the_event;
   
    } else if (status.Get_tag() == GVT_MESSAGE ) {
        // For now this must be a GVT message.
        ASSERT ( status.Get_tag() == GVT_MESSAGE );
        // Type cast does the trick as GVT messages are binary blobs
        GVTMessage *msg = reinterpret_cast<GVTMessage*>(incoming_event);
        // Let the GVT manager handle it.
        gvtManager->recvGVTMessage(msg);
    }
    
    return NULL;
}//end receiveEvent


bool
Communicator::isAgentLocal(AgentID id){
    //cout << "Check if Local Agent: " << id <<endl;
    //ASSERT(id >= 0 && id <= 4);
    SimulatorID my_id = MPI::COMM_WORLD.Get_rank();
    return (agentMap[id] == my_id);
}

void
Communicator::finalize() {
    try{
        // cout << "[COMMUNICATOR] - before MPI::finalize()" << endl;
        MPI::Finalize();
        // cout << "[COMMUNICATOR] - MPI in CommManager has been finalized." << endl;
    }catch (MPI::Exception e) {
        cerr << "MPI ERROR (finalize): ";cerr << e.Get_error_string() << endl;
    }
}

void
Communicator::setGVTManager(GVTManager *gvtMgr) {
    gvtManager = gvtMgr;
}

SimulatorID
Communicator::getOwnerRank(const AgentID &id) const {
    // Find iterator for given agent id
    AgentIDSimulatorIDMap::const_iterator entry = agentMap.find(id);
    // If id is valid then iterator is valid.
    if (entry != agentMap.end()) {
        return entry->second;
    }
    // Invalid or unknown agent id.
    return -1u;
}

void
Communicator::getProcessInfo(unsigned int &rank, unsigned int& numProcesses) {
    rank         = MPI::COMM_WORLD.Get_rank();
    numProcesses = MPI::COMM_WORLD.Get_size();
}

Communicator::~Communicator(){}

#endif
