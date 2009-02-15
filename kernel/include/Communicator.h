#ifndef _MUSE_COMMUNICATOR_H_
#define _MUSE_COMMUNICATOR_H_

#include "DataTypes.h"
#include <map>

//these are the tag types
#define AGENT_LIST      0
#define EVENT           1
#define GVT_MESSAGE     2

//these are the source types
#define ROOT_KERNEL     0


BEGIN_NAMESPACE(muse);

class GVTMessage;
class GVTManager;

class Communicator {

    //class Event;

public:
    /** The ctor.
     */
    Communicator();

    /** The dtor.
     */
    ~Communicator();

    /** The sendEvent method.
     *
     * @param Event pointer, this is the event to be sent across the wire.
     * @param int eventSize, simply use the sizeof() method of the event class
     *        you are sending on the wire.
     *        USAGE EXAMPLE:: using the clock example!
     *              ClockEvent *e = new ClockEvent();
     *              sendEvent(e, sizeof(*e)); //NOTE::that you want the sizeof what the pointer points to!!
     *                 .....
     *              free(e); //please remember to free your memory :-)
     * @see Event
     *
     */
    void sendEvent(Event *, const int);

    /** Method to send out a GVT message.

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
     *
     * NOTE:: Will return a NULL if there is no Event to receive!!!
     * @return Event pointer, this is the event to be received from the wire.
     * @see Event
     */
    Event* receiveEvent();

    /** The initialize method
     * this is used to sync all communicator and get the agentMap
     * populated.
     */
    SimulatorID initialize(int argc, char* argv[]);

    /**The registerAgents method.
     * This is used to sync the AgentMapping in the Communicator class.
     * Before you can send any events across the wire (MPI), use this to
     * get your agents and other kernel's agents mapped correctly.
     *
     * NOTE:: please call the initialize method before calling this method.
     *
     *@param AgentContainer reference, this is the list of agents the kernel containts
     */
    void registerAgents(AgentContainer &);

    /**The isAgentLocal method.
     * This will check if the given agent is registered locally.
     * @param id , the agent id is used to check. true if it is local.
     * @see AgentID
     */
    bool isAgentLocal(AgentID &);

    /** Method to obtain the rank of the process on which a given
        agent resides.

        This method can be used to determine the rank of the
        remote process on which a given agent resides.

        \note The values returned by this method make sense only
        after the communicator has been initialized and
        information regarding all the agents has been exchanged.

        \param[in] id The ID of the agent for which the
        corresponding process rank is desired.

        \return If the id (parameter) is valid then this method
        returns the rank of the process on which the given agent
        resides.  Otherwise this method returns -1.
    */
    unsigned int getOwnerRank(const AgentID &id) const;

    /** Method to obtain process configuration information.

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
    void getProcessInfo(unsigned int &rank, unsigned int& numProcesses);

    /** Set a reference to the GVT manager.

        This method must be used to set a valid pointer to the GVT
        manager class assocaited with this simulation. The GVT manager
        needs to see and process all incoming and outgoing events on
        this process.  This pointer comes in handy to interact with
        the GVT manager.  This pointer is set by the Simulator just
        before it starts simulation.

        \param[in,out] gvtMgr The GVT manager object to be used for
        processing incoming and outgoing events.
    */
    void setGVTManager(GVTManager *gvtMgr);
    
    /** The finialize method.
     *
     * Use this to clean up after yourself.
     *
     */
    void finalize();

private:

    /** This is used to map the locations of all agents in the simulation.
     *  When simulation starts, all simulation kernels, will perform a bcast and
     *  send all agents that they have. Using the agentID as the key, the communicator
     *  will be able to know the simulator kernel's ID.
     *
     * @see AgentID
     * @see SimulatorID
     */
    map<AgentID, SimulatorID> agentMap;

    /** Instance variable to hold reference to GVT manager.

        This instance variable is used to hold a pointer to the GVT
        manager associated with this simulation.  The GVT manager
        needs to see and process all incoming and outgoing events on
        this process.  This pointer comes in handy to interact with
        the GVT manager.  This pointer is set by the Simulator just
        before it starts simulation.  
    */
    GVTManager *gvtManager;
};

END_NAMESPACE(muse)

#endif
