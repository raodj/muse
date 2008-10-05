#ifndef _MUSE_EVENT_H_
#define _MUSE_EVENT_H_


//---------------------------------------------------------------------------
// USE AS IS, muse will not be responsible if anything breaks, while using muse.
// Authors: Meseret Gebre       gebremr@muohio.edu
//
//---------------------------------------------------------------------------

#include "Time.h"
#include "Agent.h"
/** The muse namespace.
 * Everything in the api is in the muse namespace.
 *
 */
namespace muse { //begin namespace declaration 

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
public:
    
        /** The ctor method.
         * This is the fastest way to create an Event object. The AgentID of the agent receiving the event and the delivery time
         * is the least amount of information needed to be a vaild Event.
         *
         *@param id this is of type AgentID and its a reference
         *@param deliveryTime this is of type Time which is also a reference. 
         */
	explicit Event(muse::AgentID &id, Time & time);
	~Event();
        
private:
    //The agent receiving the event
	AgentID receiverAgentID;
    //the time for which this event should be delivered.
    Time deliveryTime, sentTime; 
};

}//end namespace declaration

#endif
