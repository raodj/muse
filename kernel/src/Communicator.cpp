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
// Authors: Meseret R. Gebre       meseret.gebre@gmail.com
//          Dhananjai M. Rao       raodm@muohio.edu
//          Alex Chernyakhovsky    alex@searums.org  
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

Communicator::Communicator() {
    gvtManager = NULL;
}

SimulatorID
Communicator::initialize(int argc, char* argv[]) {
    MPI::Init(argc, argv); 
    return MPI::COMM_WORLD.Get_rank();
}

void
Communicator::registerAgents(AgentContainer& allAgents) {
    const int procCount = MPI::COMM_WORLD.Get_size();
    const int rank      = MPI::COMM_WORLD.Get_rank();
    
    // Add all of the local agents to the map
    AgentContainer::iterator ac_it;
    for (ac_it = allAgents.begin(); (ac_it != allAgents.end()); ac_it++) {
        agentMap[(*ac_it)->getAgentID()] = rank;
    }

    // If the number of Processes in the system is 1, we don't have any
    // registrations coming in from the network
    if (procCount == 1) {
        return;
    }

    if (rank == ROOT_KERNEL) {
        // The Root Kernel (rank 0) needs to accept registrations from
        // other processes.
        MPI::Status status;
        // We start at 1 because we have already self-registered
        for (int p = 1; (p < procCount); p++) {
            // Probe for a message from another simulation kernel.
            MPI::COMM_WORLD.Probe(MPI_ANY_SOURCE, AGENT_LIST, status);
            // Figure out the agent list size
            int agentListSize = status.Get_count(MPI::UNSIGNED);
            // Make a large enough array
            unsigned int agentList[agentListSize];
            // Perform the actual receive operation
            MPI::COMM_WORLD.Recv(&agentList, agentListSize,
                                 MPI::UNSIGNED, MPI_ANY_SOURCE,
                                 AGENT_LIST, status);
            // Add the contents of this list to master AgentMap
            for(int i = 0; (i < agentListSize); i++) {
                ASSERT(agentMap.find(agentList[i]) == agentMap.end());
                agentMap[agentList[i]] = status.Get_source();
            }
        }

        // Calculate the size of a flattened agent map. We need to
        // double the size because we are sending both the agent-id
        // and the rank
        int flatAgentMapSize  = agentMap.size() * 2;
        AgentID* flatAgentMap = new AgentID[flatAgentMapSize];
        
        int counter = 0;
        AgentIDSimulatorIDMap::iterator it = agentMap.begin();
        for (; (it != agentMap.end()); it++){
            flatAgentMap[counter]     = it->first;
            flatAgentMap[counter + 1] = it->second;
            counter += 2;
        }
        // Broadcast the size of the flat agent map
        MPI::COMM_WORLD.Bcast(&flatAgentMapSize, 1, MPI::INT, ROOT_KERNEL);
        // Broadcast the actual flat agent map
        MPI::COMM_WORLD.Bcast(flatAgentMap, flatAgentMapSize, MPI::UNSIGNED,
                              ROOT_KERNEL);
        std::cout << "Agent Registration: complete!" << std::endl;
        delete[] flatAgentMap;
    } else {
        // Other processes need to send their registrations along.
        AgentID agentList[allAgents.size()];
        // Build the flat agent list
        for (size_t i = 0; i < allAgents.size(); i++) {
            agentList[i] = allAgents[i]->getAgentID();
        }
        // Send the flat list across with MPI
        MPI::COMM_WORLD.Send(agentList, allAgents.size(), MPI::UNSIGNED,
                             ROOT_KERNEL, AGENT_LIST);
        //get the size of incoming agentMap list
        int agentMapLength = 0;
        MPI::COMM_WORLD.Bcast(&agentMapLength, 1, MPI::INT, ROOT_KERNEL);
        
        // Receive a completed agentMap from the Root Kernel (rank 0)
        AgentID* flatAgentMap = new AgentID[agentMapLength];
        MPI::COMM_WORLD.Bcast(flatAgentMap, agentMapLength, MPI::UNSIGNED,
                              ROOT_KERNEL);

        // Populate the real agentMap using the flatAgentMap
        ASSERT(agentMapLength % 2 == 0);
        for (int i = 0; (i < agentMapLength); i += 2){
            agentMap[flatAgentMap[i]] = flatAgentMap[i + 1];
        }

        delete[] flatAgentMap;
    }
}

void
Communicator::sendEvent(Event* e, const int eventSize){
    try {
        // Send event as raw (char) data
        char* serialEvent = reinterpret_cast<char*>(e);
        MPI::COMM_WORLD.Send(serialEvent, eventSize, MPI::CHAR,
                             agentMap[e->getReceiverAgentID()], EVENT);
        //cout << "SENT an Event of size: " << eventSize << endl;
        //cout << "[COMMUNICATOR] - made it in sendEvent" << endl;
        //e->decreaseReference();
    } catch (MPI::Exception e) {
        std::cerr << "MPI ERROR (sendEvent): "
                  << e.Get_error_string() << std::endl;
    }
}

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
    // Figure out the agent list size
    int eventSize = status.Get_count(MPI::CHAR);
    char *incoming_event = new char[eventSize];
    // Read the actual data.
    try {
        MPI::COMM_WORLD.Recv(incoming_event, eventSize, MPI::CHAR,
                             status.Get_source(), status.Get_tag(), status);
    } catch (MPI::Exception& e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        delete[] incoming_event;
        return NULL;
    }

    // Ensure some of the core data is valid.        
    ASSERT( gvtManager != NULL );
    
    // Now handle the incoming data based on the tag value.
    if (status.Get_tag() == EVENT) {
        // Type cast does the trick as events are binary blobs
        Event* the_event = reinterpret_cast<Event*>(incoming_event);
        // Since the event is from the network, the reference count must be 1
        ASSERT(the_event->getReferenceCount() == 1);
        // Let GVT manager inspect incoming events.
        gvtManager->inspectRemoteEvent(the_event);
        // Dispatch event for further processing.
        return the_event;
    } else if (status.Get_tag() == GVT_MESSAGE ) {
        // Type cast does the trick as GVT messages are binary blobs
        GVTMessage *msg = reinterpret_cast<GVTMessage*>(incoming_event);
        // Let the GVT manager handle it.
        gvtManager->recvGVTMessage(msg);
    }
    
    return NULL;
}

bool
Communicator::isAgentLocal(const AgentID id) {
    //cout << "Check if Local Agent: " << id <<endl;
    //ASSERT(id >= 0 && id <= 4);
    SimulatorID my_id = MPI::COMM_WORLD.Get_rank();
    return (agentMap[id] == my_id);
}

void
Communicator::finalize() {
    try {
        // cout << "[COMMUNICATOR] - before MPI::finalize()" << endl;
        MPI::Finalize();
        // cout << "[COMMUNICATOR] - MPI in CommManager has been finalized." << endl;
    } catch (MPI::Exception e) {
        std::cerr << "MPI ERROR (finalize): "
                  << e.Get_error_string() << std::endl;
    }
}

void
Communicator::setGVTManager(GVTManager* gvtMgr) {
    gvtManager = gvtMgr;
}

SimulatorID
Communicator::getOwnerRank(const AgentID& id) const {
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
Communicator::getProcessInfo(unsigned int& rank, unsigned int& numProcesses) {
    rank         = MPI::COMM_WORLD.Get_rank();
    numProcesses = MPI::COMM_WORLD.Get_size();
}

Communicator::~Communicator() {}

#endif
