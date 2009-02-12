#ifndef _MUSE_EVENT_H_
#define _MUSE_EVENT_H_


//---------------------------------------------------------------------------
// USE AS IS, muse will not be responsible if anything breaks, while using muse.
// Authors: Meseret Gebre       gebremr@muohio.edu
//
//---------------------------------------------------------------------------


#include "DataTypes.h"

extern std::ostream& operator<<(ostream&, const muse::Event&);

/** The muse namespace.
 * Everything in the api is in the muse namespace.
 *
 */
BEGIN_NAMESPACE(muse) //begin namespace declaration

// Forward declaration for agentID to keep the compiler happy.
//typedef struct AgentIDType AgentID;


/** The base class for all events in a simulation.
 *
 * This class represents the base class from which all user-defined simulation events
 * must be derived. 
 *
 * To create an event for your simulation, make sure you derive from this Event Class, Also 
 * you will NOT override any of the given methods, instead add your own methods and whatever the
 * agent receiving the event needs to process the event.
*/
class Event {
    friend std::ostream& ::operator<<(ostream&, const muse::Event&);
    friend class Agent;
    friend class Simulation;
    friend class Scheduler;
public:
    
        /** The ctor method.
         * This is the fastest way to create an Event object. The AgentID of the agent receiving the event and the delivery time
         * is the least amount of information needed to be a vaild Event.
         *
         *@param id this is of type AgentID and its a reference
         *@param deliveryTime this is of type Time which is also a reference. 
         */
	 explicit Event(const AgentID  senderID,const AgentID  receiverID,
                   const Time  sentTime, const Time  receiveTime);

    const AgentID & getSenderAgentID() const;

    const AgentID & getReceiverAgentID() const;

    const Time & getSentTime() const;

    const Time & getReceiveTime() const;


    
    //protected:
        /**The decreaseReference method.
     *
     */
    void decreaseReference();

    /**The increaseReference method.
     *
     */
    void increaseReference();

   

    bool isAntiMessage() const;

    int getReferenceCount();
    void makeAntiMessage();
    ~Event(); //this will prevent from destroying

    AgentID senderAgentID, receiverAgentID;
    Time sentTime,receiveTime;
    bool antiMessage;      //this is used to let the scheduler that this event is an anti-message
 
    int referenceCount; //this is for memory managements
};

END_NAMESPACE(muse)//end namespace declaration

#endif
