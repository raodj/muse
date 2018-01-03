#ifndef MUSE_MULTI_THREADED_SHM_COMMUNICATOR_H
#define MUSE_MULTI_THREADED_SHM_COMMUNICATOR_H

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <mutex>
#include "Communicator.h"

BEGIN_NAMESPACE(muse);

// Forward declarations to keep compile fast
class GVTMessage;
class MultiThreadedShmSimulationManager;

// A short cut to a vector of unsigned integers
using VecUint = std::vector<unsigned int>;

/** Communicator for managing multi-threaded access to MPI-based
    communications.

    This class extends the base class to further customize the
    operation of the communicator to facilitate inter-thread
    communication as well as inter-node communication via MPI.
*/
class MultiThreadedShmCommunicator : public muse::Communicator {
    // The manager needs to add information on agents
    friend class MultiThreadedShmSimulationManager;
public:
    /** \brief Default Constructor.

        Initialize the Communicator with core information.

        \param[in] simMgr The simulation manager associated with this
        communicator.  This pointer is internally saved and used to
        enqueue incoming events (via call to
        MultiThreadedSimulationManager::addIncomingEvent method) from
        local threads or over MPI.  This parameter cannot be NULL.
        
        \param[in] threadsPerNode The number of threads being used on
        each node.  This information is used for routing events and
        GVT messages.
    */
    MultiThreadedShmCommunicator(MultiThreadedShmSimulationManager* simMgr,
                              const int threadsPerNode);

    /** \brief Destructor

        Do nothing, since no memory is allocated.
    */
    virtual ~MultiThreadedShmCommunicator() override;

    /** \brief Sync all communicators and get the agentMap populated.

	\param[in,out] argc the number of arguments to pass to MPI::Init
        
	\param[in,out] argv the list of arguments to pass to MPI:Init

        \param[in] initMPI Flag to indicate if MPI needs to be
        reinitialized.  This flag is set to false if the simulation is
        simply being repeated and initialization of MPI is not
        necessary.
        
	\return SimulatorID (MPI rank of the process)

	\see SimulatorID
    */
    virtual SimulatorID initialize(int argc, char* argv[],
                                   bool initMPI) override;
    
    /** \brief Clean up after yourself

        \param[in] stopMPI If this flag is true, then MPI is finalized
        via call to MPI calls the MPI:Finalize.  Otherwise MPI is not
        finalized permitting another simulation run using current
        setup.
    */
    virtual void finalize(bool stopMPI = true) override;

    /** Determine the thread ID for the specified agent.

        This method can be used to determine the thread ID associated
        with a given agent.

        \param[in] id The ID of the agent whose thread ID is
        desired. Calling this method with an invalid or non-local
        agent results in a return value of -1.

        \return The ID of the thread or -1 (if the agent ID is not
        local or invalid).
    */
    int getThreadID(const AgentID id) const {
        AgentIDSimulatorIDMap::const_iterator entry = agentThreadMap.find(id);
        return (entry != agentThreadMap.end() ? (int) entry->second : -1);
    }

    /** Determine the thread ID for the specified agent.

        This method can be used to determine the thread ID associated
        with a given agent.

        \param[in] id The ID of the agent whose thread ID is
        desired.

        \param[in] defaultThrID The default value to be returned if
        the specified agent is not a local agent.

        \return The ID of the thread or defaultThrID (if the agent ID
        is not local or invalid.
    */
    int getThreadID(const AgentID id, const int defaultThrID) const {
        AgentIDSimulatorIDMap::const_iterator entry = agentThreadMap.find(id);
        ASSERT((defaultThrID >= 0) && (defaultThrID < threadsPerNode));
        return (entry != agentThreadMap.end() ? (int) entry->second :
                defaultThrID);
    }
    
    /** \brief Obtain the thread-based rank of the process on which a
        given agent resides.

        This method can be used to determine the thread-based rank,
        that is: <i>getOwnerRank(id) * getThreadID(id)</i> of the
        remote process on which a given agent resides.  This is used
        by GVT manager to forward tokens between threads.

        \note The values returned by this method make sense only after
        the communicator has been initialized and information
        regarding all the agents has been exchanged.

        \param id The ID of the agent for which the corresponding
        thread-based rank is desired.

        \return If the id (parameter) is valid then this method
        returns the rank of the process on which the given agent
        resides.  Otherwise this method returns a negative value.
    */
    virtual int getOwnerRank(const AgentID& id) const override {
        return (Communicator::getOwnerRank(id) / threadsPerNode);
    }

    /** \brief Obtain the thread-based rank of the process on which a
        given agent resides.

        This method can be used to determine the thread-based rank,
        that is: <i>getOwnerRank(id) * getThreadID(id)</i> of the
        thread on which a given agent resides.  This information is
        directly obtained from the agentMap which already contains
        agent--thread mapping.

        \note The values returned by this method make sense only after
        the communicator has been initialized and information
        regarding all the agents has been exchanged.

        \param id The ID of the agent for which the corresponding
        thread-based rank is desired.

        \return If the id (parameter) is valid then this method
        returns the rank of the process on which the given agent
        resides.  Otherwise this method returns a negative value.
    */
    virtual int getOwnerThreadRank(const AgentID& id) const override {
        // Find iterator for given agent id
        AgentIDSimulatorIDMap::const_iterator entry = agentMap.find(id);
        // If id is valid then iterator is valid.
        return (entry != agentMap.end()) ? (int) entry->second : -1;
    }
    
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

        \param[out] totNumThreads The number of threads, that is --
        <i>processes * threadsPerProcess</i> in the parallel,
        distributed simulation.  The default base class always returns
        the totNumThreads to be the same as numProcesses.
    */
    virtual void getProcessInfo(unsigned int& rank, unsigned int& numProcesses,
                                unsigned int& totNumThreads);

    /** \brief Send the specified Event to the appropriate remote
        process or thread.

        This method overrides the default implementation in the base
        class to enable sending events between threads via
        shared-memory MT-safe queues.
        
	\param[in] e The event to be sent across the wire
        
	\param[in] eventSize The size (in bytes) of the event to send
    */
    virtual void sendEvent(Event* e, const int eventSize) override;

    /** \brief Send out a GVT message.

        This method must be used to dispatch a GVT message from this
        process to another process.  This method is typically invoked
        only from the GVTManager class.  This method handles sending
        message to threads on local node.

        \param[in] msg The message to be dispatched to a remote
        process.

        \param[in] destRank The rank of the destination process to
        which the event is to be dispatched.
    */
    virtual void sendMessage(const GVTMessage *msg,
                             const int destRank) override;

    /** \brief Send out a string as a message with a given tag.

        This method is used to dispatch a generic string as a message
        from this process to another process.  This method is
        typically invoked only from the Simulator class hierarcy.
        This method overrides the base class implementation to ensure
        calls from multiple threads are serialized (using a mutex).

        \param[in] string The string to be dispatched to a remote
        process. The string can be an empty string.

        \param[in] destRank The rank of the destination process to
        which the event is to be dispatched.

        \param[in] tag An optional tag with which the messages must be
        tagged for dispatch.  This is a convenience feature that can
        be used to send different types of messages.
    */
    virtual void sendMessage(const std::string& str, const int destRank,
                             int tag = STRING_MESSAGE) override;
    
    /** \brief Retrieve a string from a message.

        This method is used to read a generic string from a message
        send from another process.  This method assumes that the
        message being read was sent via a call to the sendMessage()
        method in this class.  This method is typically invoked only
        from the Simulator class hierarchy.  This method overrides the
        base class implementation to ensure calls from multiple
        threads are serialized (using a mutex).


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
    virtual std::string receiveMessage(int& recvRank,
                                       const int srcRank = MPI_ANY_SOURCE,
                                       int tag = STRING_MESSAGE,
                                       bool blocking = true) override;

    /** The recvEvent method to read one event via an MPI call.

        \note This method is not used with working with multiple
        threads as constantly locking/unlocking mpiMutex was found to
        be very ineffective when there is high volume of
        communications.  Instead the receiveManyEvents method is used.
        
        This method overrides the base class implementation to ensure
        calls from multiple threads are serialized (using a mutex).
        The actual reading of event is done via call to
        readOneMessage() method in this class.

	\return Event pointer, this is the event to be received from
	the wire.  This method will return a NULL if an event was not
	received.
    */
    virtual Event* receiveEvent() override;

    /** Helper emthod to recvEvent one event via an MPI call.

        \note This method assumes that mpiMutex has already been
        locked before it is invoked.

        This method is a refactored method that is called from
        receiveEvent() or receiveManyEvents() methods.  This method
        performs the interactions with MPI invokving probing for
        messages and reading any pending message.

	\return Event pointer, this is the event to be received from
	the wire.  This method will return a NULL if an event was not
	pending to be read.  This method also returns NULL on
	exceptions.
    */
    Event* receiveOneEvent();
    
    /** The recvEvent method to read one event via an MPI call.

        This method is repeatedly invoked from the core simulation
        loop to read a given number of events received over the wire
        from MPI.  This method is invoked only from
        MultiThreadedSimulationManager::processMpiMsgs() method.

        \param[out] eventList The event list into received events are
        to be placed.  Events are appended to this list.  Existing
        entries are unaltered.

        \param[in] maxEvents The maximum number of events to
        read/process.

        \param[in] retryCount The number times to try to get mpiMutex
	lock before giving up.

        \return Returns the number of events read.  If no events were
        ready then this method returns 0 (zero).
    */
    int receiveManyEvents(EventContainer& eventList, const int maxEvents,
                          int retryCount = 10);
    
protected:
    /** Set/update the logical thread ID for an agent on this physical
        process.

        This method is invoked as agents are registered and added to
        the differen threads on this physical process.  This method
        updates entries in the agentThreadMap to enable looking-up
        thread ID's associated with a given agent.

        \param[in] id The ID of the agent

        \param[in] thrIdx The zero-based index of the thread that is
        actually managing the agent.
    */
    void setAgentThread(const AgentID id, const int thrIdx) {
        agentThreadMap[id] = thrIdx;
    }

    /** Registers local agents (already in agentThreadMap) with all
        processes.

        This method creates a list of local agents and uses
        Communicator::registerAgents(const std::vector<AgentID>&)
        method to register agents with all the parallel processes.
    */
    void registerAllAgents();

private:
    /** Method run on MPI-rank zero to aggregate and broadcast
        agent--thread mapping.

        This method is invoked from the registerAgents() method in
        this class only on rank zero.  This method first gathers the
        agent--thread mapping from all other processes into the
        supplied idThrList.  It then broadcasts the aggregate list to
        all other processes.

        \param[in,out] idThrList The list of local mappings.  This
        list is modified by this method to contain the full list of
        mappings.
    */
    void gatherBroadcastAgentInfo(VecUint& idThrList);

    /** Method run on MPI-rank not-zero to send and receive
        agent--thread mapping.

        This method is invoked on all processes that are not rank zero
        to coordinate activites in the gatherBroadcastAgentInfo
        method.  This first sends the local agent--thread mapping to
        rank zero.  It then receives the broadcasted aggregate list
        information.

        \param[in,out] idThrList The list of local mappings.  This
        list is modified by this method to contain the full list of
        mappings.        
    */
    void sendGatherAgentInfo(VecUint& idThrList);
                          
private:
    /** \brief The zero-based thread IDs on this local process that
        logically manages a given agent.

        As local agents are registered, the logical threads that
	manages the agent are added to this map.  This information is
	used to route local and incoming MPI events to appropriate
	threads.
    */
    AgentIDSimulatorIDMap agentThreadMap;

    /** The simulation manager associated with this
        communicator.

        This pointer is internally saved and used to enqueue incoming
        events (via call to
        MultiThreadedSimulationManager::addIncomingEvent method) from
        local threads or over MPI.  The value is set in the
        constructor and is never changed during the life time of this
        object.
    */
    MultiThreadedShmSimulationManager* const simMgr;

    /** The number of threads to be spun-up for each MPI process.

        This value is initialized in the constructor and is never
        changed after that.  The actual value to use is set by
        MultiThreadedSimulationManager::initialize() method.
    */
    const int threadsPerNode;

    /** A mutex to ensure exclusive calls to MPI routines.

        This mutex is used in various methods in this class to ensure
        that calls to MPI are serialized to ensure that MPI is used in
        a MT-safe manner.  This mutex is not necessary if the
        underlying MPI implementation supports multiple threads (most
        new ones do) but this is currently used as the default to
        ensure widest range of compatibility.
    */
    std::mutex mpiMutex;
    
    /** The total numebr of MPI processes constituting this
        distributed simulation.

        This instance variable is set to the actual number of MPI
        processes in the simulation.  It is used in sendMessage method
        to streamline sending messages based on thread-based rank
        (specifically to disambiguate if 0 rank is on a different
        process or same process).
    */
    unsigned int numMpiProcesses;
};

END_NAMESPACE(muse)

#endif
