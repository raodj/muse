#ifndef MUSE_COMMUNICATOR_H
#define MUSE_COMMUNICATOR_H

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

#include "DataTypes.h"
#include "HashMap.h"

//these are the tag types
#define AGENT_LIST        0
#define EVENT             1
#define GVT_MESSAGE       2
#define GVT_ESTIMATE_TIME 3

//these are the source types
#define ROOT_KERNEL       0

BEGIN_NAMESPACE(muse);

class GVTMessage;
class GVTManager;

class Communicator {
public:
    /** \brief Default Constructor.

        Initialize the Communicator without a GVTManager.
        
    */
    Communicator();

    /** \brief Destructor

        Do nothing, since no memory is allocated.
    */
    ~Communicator();

    /** \brief Send the specified Event to the appropriate remote
        process
        
	\param[in] e The event to be sent across the wire
	\param[in] eventSize The size (in bytes) of the event to send
        
        Example usage:
        \code
	ClockEvent* e = new ClockEvent();
	sendEvent(e, sizeof(*e)); 
        \endcode
        
	\see Event
    */
    void sendEvent(Event* e, const int eventSize);

    /** \brief Send out a GVT message.

        This method must be used to dispatch a GVT message from
        this process to another process.  This method is typically
        invoked only from the GVTManager class.

        \param[in] msg The message to be dispatched to a remote
        process.

        \param[in] destRank The rank of the destination process to
        which the event is to be dispatched.
    */
    void sendMessage(const GVTMessage *msg, const int destRank);
	
    /** The recvEvent method.
	
        \note This method will return a NULL if there is no Event to
        receive
        
	\return Event pointer, this is the event to be received from
	the wire.

        \see Event
    */
    Event* receiveEvent();

    /** \brief Sync all communicators and get the agentMap populated.

	\param argc the number of arguments to pass to MPI::Init
	\param argv the list of arguments to pass to MPI:Init

	\return SimulatorID (MPI rank of the process)

	\see SimulatorID
    */
    SimulatorID initialize(int argc, char* argv[]);

    /** \brief Synchronize AgentMapping between Kernel and
        Communicator
        
	This is used to sync the AgentMapping in the Communicator class.
	Before you can send any events across the wire (MPI), use this to
	get your agents and other kernel's agents mapped correctly.
     
	\note initialize() must be called before this method
     
	\param[in,out] allAgents The list of agents the kernel contains

        \see AgentContainer
    */
    void registerAgents(AgentContainer& allAgents);

    /** \brief Check if the given agent is registered locally.
	
	\param[in] id The agent id is used to check

        \return True if the agent with the specified ID is local
        
	\see AgentID
    */
    bool isAgentLocal(const AgentID id);

    /** \brief Obtain the rank of the process on which a given
        agent resides.

        This method can be used to determine the rank of the
        remote process on which a given agent resides.

        \note The values returned by this method make sense only
        after the communicator has been initialized and
        information regarding all the agents has been exchanged.

        \param id The ID of the agent for which the
        corresponding process rank is desired.

        \return If the id (parameter) is valid then this method
        returns the rank of the process on which the given agent
        resides.  Otherwise this method returns -1.
    */
    unsigned int getOwnerRank(const AgentID& id) const;

    /** \brief Obtain process configuration information.

        This method may be used to obtain some of the
        configuration information associated with the processes
        participating in the simulation.  This method is used for
        example in GVTManager::initialize() to determine
        simulation configuration.

        \note The values returned by this method make sense only
        after the communicator has been initialized and
        information regarding all the agents has been exchanged.
        Invoking this method before the Communicator has been
        initialized will result in undefined behavior.

        \param[out] rank The rank of the process on which this
        method is being invoked.

        \param[out] numProcesses The total number of processes
        constituting the simulation.
    */
    void getProcessInfo(unsigned int& rank, unsigned int& numProcesses);

    /** \brief Set a reference to the GVT manager.

        This method must be used to set a valid pointer to the GVT
        manager class assocaited with this simulation. The GVT manager
        needs to see and process all incoming and outgoing events on
        this process.  This pointer comes in handy to interact with
        the GVT manager.  This pointer is set by the Simulator just
        before it starts simulation.

        \param[in,out] gvtMgr The GVT manager object to be used for
        processing incoming and outgoing events.
    */
    void setGVTManager(GVTManager* gvtMgr);
    
    /** \brief Clean up after yourself
	
	MPI calls the MPI:Finalize
    */
    void finalize();

private:

    /** \brief The locations of all agents in the simulation.
        
	When simulation starts, all simulation kernels, will perform a
	bcast and send all agents that they have. Using the agentID as
	the key, the communicator will be able to know the simulator
	kernel's ID.
	
	\see AgentID
	\see SimulatorID
    */
    AgentIDSimulatorIDMap agentMap;
    
    /** \brief Instance variable to hold reference to GVT manager.

        This instance variable is used to hold a pointer to the GVT
        manager associated with this simulation.  The GVT manager
        needs to see and process all incoming and outgoing events on
        this process.  This pointer comes in handy to interact with
        the GVT manager.  This pointer is set by the Simulator just
        before it starts simulation.  
    */
    GVTManager* gvtManager;
};

END_NAMESPACE(muse)

#endif
