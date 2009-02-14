#ifndef MUSE_EVENT_H
#define MUSE_EVENT_H

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

#include "DataTypes.h"

// Forward declaration for insertion operator for Event
extern std::ostream& operator<<(ostream&, const muse::Event&);

/** The muse namespace.
    
    Everything in the api is in the muse namespace.
    
*/
BEGIN_NAMESPACE(muse); //begin namespace declaration

/** The base class for all events in a simulation.
 
    This class represents the base class from which all user-defined
    simulation events must be derived.
 
    To create an event for your simulation, make sure you derive from
    this Event Class, Also you will NOT override any of the given
    methods, instead add your own methods and whatever the agent
    receiving the event needs to process the event.
*/
class Event {
    friend std::ostream& ::operator<<(ostream&, const muse::Event&);
    friend class Agent;
    friend class Simulation;
    friend class Scheduler;
public:
    /** The ctor method.

        This is the fastest way to create an Event object. The AgentID
        of the agent receiving the event and the delivery time is the
        least amount of information needed to be a vaild Event.
        
        @param id this is of type AgentID and its a reference
        
        @param deliveryTime this is of type Time which is also a reference. 
    */
    explicit Event(const AgentID  senderID,const AgentID  receiverID,
                   const Time  sentTime, const Time  receiveTime);

    inline const AgentID& getSenderAgentID() const {
        return senderAgentID;
    }

    inline const AgentID& getReceiverAgentID() const {
        return receiverAgentID;
    }

    inline const Time& getSentTime() const { return sentTime; }

    inline const Time& getReceiveTime() const { return receiveTime; }
    
    //protected:
    /**The decreaseReference method.
     *
     */
    void decreaseReference();

    /**The increaseReference method.
     *
     */
    void increaseReference();

    inline bool isAntiMessage() const { return antiMessage; }

    void makeAntiMessage();

private:
    ~Event(); //this will prevent from destroying
    
    AgentID senderAgentID, receiverAgentID;
    Time sentTime,receiveTime;
    bool antiMessage;      //this is used to let the scheduler that this event is an anti-message
 
    int referenceCount; //this is for memory managements

    /** Instance variable to hold the color of this event for GVT
        computation.

        This instance variable is used to hold the color of this event
        for GVT computation using Mattern's GVT algorithm.  This
        character can take one of two values: 1 (for red events) and 2
        (for white events).  This information is set whenever an event
        is created and then used by the GVT manager to track events
        for GVT computation.

        \see GVTManager
    */
    char color;         
};

END_NAMESPACE(muse); //end namespace declaration

#endif
