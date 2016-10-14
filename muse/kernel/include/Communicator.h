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
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include "MPIHelper.h"
#include "HashMap.h"

//these are the tag types
#define AGENT_LIST        0
#define EVENT             1
#define GVT_MESSAGE       2
#define GVT_ESTIMATE_TIME 3
#define STRING_MESSAGE    4

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

    /** \brief Send out a string as a message with a given tag.

        This method muse be used to dispatch a generic string as a
        message from this process to another process.  This method is
        typically invoked only from the Simulator class.

        \param[in] string The string to be dispatched to a remote
        process. The string can be an empty string.

        \param[in] destRank The rank of the destination process to
        which the event is to be dispatched.

        \param[in] tag An optional tag with which the messages must be
        tagged for dispatch.  This is a convenience feature that can
        be used to send different types of messages.
    */
    void sendMessage(const std::string& str, const int destRank,
                     int tag = STRING_MESSAGE);
    
    /** \brief Retrieve a string from a message.

        This method muse be used to read a generic string from a
        message send from another process.  This method assumes that
        the message being read was sent via a call to the
        sendMessage() method in this class.  This method is typically
        invoked only from the Simulator class.

        \param[out] recvRank The actual rank from where the message
        was received.  If the receive was non-blocking (i.e., blocking
        flag was set to false) and a string message was not pending,
        then this method sets recvRank to -1.

        \param[in] blocking If this flag is true, then blocking MPI
        operations are used.  Otherwise non-blocking operations are
        used to receive a message.
        
        \param[in] srcRank The rank of the process from where the
        event is to be read.  The rank can be MPI::ANY_SOURCE to read
        message from any process. 

        \param[in] tag An optional tag for the messages from where the
        string is to be read.  This is a convenience feature that can
        be used to read a specific type of message.

        \note Pay attention to the use of recvRank to distinguish
        between an empty string versus no string case when using
        non-blocking MPI.
        
        \return If a string was successfully read, then this method
        returns it and sets srcRank.  Otherwise srcRank is set to -1
        and this method returns an empty string.
    */
    std::string receiveMessage(int& recvRank,
                               const int srcRank = MPI_ANY_SOURCE,
                               int tag = STRING_MESSAGE, bool blocking = true);
    
    /** The recvEvent method.
	
        \note This method will return a NULL if there is no Event to
        receive
        
	\return Event pointer, this is the event to be received from
	the wire.

        \see Event
    */
    Event* receiveEvent();

    /** \brief Sync all communicators and get the agentMap populated.

	\param[in,out] argc the number of arguments to pass to MPI::Init
        
	\param[in,out] argv the list of arguments to pass to MPI:Init

        \param initMPI[in] Flag to indicate if MPI needs to be
        reinitialized.  This flag is set to false if the simulation is
        simply being repeated and initialization of MPI is not
        necessary.
        
	\return SimulatorID (MPI rank of the process)

	\see SimulatorID
    */
    SimulatorID initialize(int argc, char* argv[], bool );

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

        \param[in] stopMPI If this flag is true, then MPI is finalized
        via call to MPI calls the MPI:Finalize.  Otherwise MPI is not
        finalized permitting another simulation run using current
        setup.
    */
    void finalize(bool stopMPI = true);

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
