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
#include "DataTypes.h"
#include "Event.h"
#include "Agent.h"

using namespace muse;

Communicator::Communicator() {
    gvtManager = NULL;
}

SimulatorID
Communicator::initialize(int argc, char* argv[], bool initMPI) {
    if (initMPI) {
        // Initialize MPI.
        MPI_INIT(argc, argv);
    }
    return MPI_GET_RANK();
}

void
Communicator::registerAgents(AgentContainer& allAgents) {
    const int procCount = MPI_GET_SIZE();
    const int rank      = MPI_GET_RANK();
    
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
        MPI_STATUS status;
        // We start at 1 because we have already self-registered
        for (int p = 1; (p < procCount); p++) {
            // Probe for a message from another simulation kernel.
            MPI_PROBE(MPI_ANY_SOURCE, AGENT_LIST, status);
            // Figure out the agent list size
            int agentListSize = status.Get_count(MPI_TYPE_UNSIGNED);
            // Make a large enough array
            std::vector<unsigned int> agentList(agentListSize);
            // Perform the actual receive operation
            MPI_RECV(&agentList[0], agentListSize, MPI_TYPE_UNSIGNED,
                     status.Get_source(), AGENT_LIST, status);
            // Add the contents of this list to master AgentMap
            for(int i = 0; (i < agentListSize); i++) {
                if (agentMap.find(agentList[i]) != agentMap.end()) {
                    std::cerr << "Duplicate agent with ID: "
                              << agentList[i] << " encountered. Aborting!\n";
                    ASSERT( false );
                }
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
        MPI_BCAST(&flatAgentMapSize, 1, MPI_TYPE_INT, ROOT_KERNEL);
        // Broadcast the actual flat agent map
        MPI_BCAST(flatAgentMap, flatAgentMapSize, MPI_TYPE_UNSIGNED,
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
        MPI_SEND(agentList, allAgents.size(), MPI_TYPE_UNSIGNED,
                 ROOT_KERNEL, AGENT_LIST);
        //get the size of incoming agentMap list
        int agentMapLength = 0;
        MPI_BCAST(&agentMapLength, 1, MPI_TYPE_INT, ROOT_KERNEL);        
        // Receive a completed agentMap from the Root Kernel (rank 0)
        AgentID* flatAgentMap = new AgentID[agentMapLength];
        MPI_BCAST(flatAgentMap, agentMapLength, MPI_TYPE_UNSIGNED, ROOT_KERNEL);
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
        MPI_SEND(serialEvent, eventSize, MPI_TYPE_CHAR,
                 agentMap[e->getReceiverAgentID()], EVENT);
        //cout << "SENT an Event of size: " << eventSize << endl;
        //cout << "[COMMUNICATOR] - made it in sendEvent" << endl;
        //e->decreaseReference();
    } catch (const MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (sendEvent): "
                  << e.Get_error_string() << std::endl;
    }
}

void
Communicator::sendMessage(const GVTMessage *msg, const int destRank) {
    try {
        // GVT messages are already serialized.
        const char *data = reinterpret_cast<const char*>(msg);
        MPI_SEND(data, msg->getSize(), MPI_TYPE_CHAR, destRank, GVT_MESSAGE);
    } catch (const MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (sendMessage): ";
        std::cerr << e.Get_error_string() << std::endl;
    }
}

void
Communicator::sendMessage(const std::string& str, const int destRank, int tag) {
    try {
        MPI_SEND(str.c_str(), str.size() + 1, MPI_TYPE_CHAR, destRank, tag);
    } catch (const MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (sendMessage): ";
        std::cerr << e.Get_error_string() << std::endl;
    }
}

std::string
Communicator::receiveMessage(int& recvRank, const int srcRank, int tag,
                             bool blocking) {
    recvRank = -1;  // Initialize to invalid value
    MPI_STATUS status;
    try {
        if (!blocking && !MPI_IPROBE(srcRank, tag, status)) {
            // No pending message.
            return "";
        } else if (blocking) {
            // Wait until we get a valid message to read.
            MPI_PROBE(srcRank, tag, status);
        }
    } catch (const MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        return NULL;
    }
    // Figure out the size of the string we need.
    const int strSize = status.Get_count(MPI_TYPE_CHAR);
    std::string msg(strSize, 0);
    // Read the actual string data.
    try {
        MPI_RECV(&msg[0], strSize, MPI::CHAR,
                 status.Get_source(), status.Get_tag(), status);
        recvRank = status.Get_source();
    } catch (MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        return "";
    }
    return msg;
}

Event*
Communicator::receiveEvent(){
    MPI_STATUS status;
    try {
        if (!MPI_IPROBE(MPI_ANY_SOURCE, MPI_ANY_TAG, status)) {
            // No pending event.
            return NULL;
        }
    } catch (const MPI_EXCEPTION& e) {
        std::cerr << "MPI ERROR (receiveEvent): ";
        std::cerr << e.Get_error_string() << std::endl;
        return NULL;
    }
    // Figure out the agent list size
    int eventSize = status.Get_count(MPI_TYPE_CHAR);
    char *incoming_event = new char[eventSize];
    // Read the actual data.
    try {
        MPI_RECV(incoming_event, eventSize, MPI_TYPE_CHAR,
                 status.Get_source(), status.Get_tag(), status);
    } catch (const MPI_EXCEPTION& e) {
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
    SimulatorID my_id = MPI_GET_RANK();
    return (agentMap[id] == my_id);
}

void
Communicator::finalize(bool stopMPI) {
    try {
        if (stopMPI) {
            MPI_FINALIZE();
        }
    } catch (const MPI_EXCEPTION& e) {
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
    rank         = MPI_GET_RANK();
    numProcesses = MPI_GET_SIZE();
}

Communicator::~Communicator() {}

#endif
