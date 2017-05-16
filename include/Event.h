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
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include "DataTypes.h"
#include <unordered_map>
#include <stack>

// Forward declaration for insertion operator for Event
extern std::ostream& operator<<(std::ostream&, const muse::Event&);

/** \def CREATE(EventClassName, int size, receiverID, recvTime)
    
    \brief A convenience macro to facilitate the definition of MUSE
    API compliant static create method for instantiating events.
    
    This macro must be used in each class that is derived from this
    Event class to implement a suitate create method.
*/
#define CREATE(EventClassName)                                          \
    static EventClassName* create(const muse::AgentID receiverID,       \
                                  const muse::Time recvTime,            \
                                  const int size = sizeof(EventClassName)) { \
        EventClassName* event =                                         \
            reinterpret_cast<EventClassName*>(allocate(size));          \
        new (event) EventClassName(receiverID, recvTime);               \
        return event;                                                   \
    }

BEGIN_NAMESPACE(muse);

/** The base class for all events in a simulation.
    This class represents the base class from which all user-defined
    simulation events must be derived.
    
    To create an event for your simulation, make sure you derive from
    this Event Class, Also you will NOT override any of the given
    methods, instead add your own methods and whatever the agent
    receiving the event needs to process the event.
*/
class Event {
    friend std::ostream& ::operator<<(std::ostream&, const muse::Event&);
    friend class Agent;
    friend class Simulation;
    friend class Scheduler;
    friend class GVTManager;
    friend class Communicator;
    friend class BinaryHeapWrapper;
    friend class ListBucket;
    friend class VectorBucket;
    friend class TwoTierBucket;
    friend class TwoTierBottom;
    friend class OneTierBottom;
    friend class TwoTierLadderQueue;    
    friend class LadderQueue;
    friend class Bottom;    
    friend class HeapBottom;
    friend class MultiSetBottom;
    friend class HeapEventQueue;
    friend class BinomialHeapEventQueue;
    friend class TwoTierHeapEventQueue;
    friend class ThreeTierHeapEventQueue;
    friend class TwoTierHeapAdapter;
    friend class TwoTierHeapOfVectorsEventQueue;
    friend class AgentPQ;
public:
    // Use MUSE macro to define the create method to instantiate
    // this event class.
    CREATE(Event);

    /** \brief Get the ID of the Agent that sent this Event
        
        \return The sender AgentID.
        \see AgentID
    */
    inline AgentID getSenderAgentID() const { return senderAgentID; }

    /** \brief Get the ID of the Agent that will receive this Event
        
        \return The receiver AgentID.
        \see AgentID
    */
    inline AgentID getReceiverAgentID() const { return receiverAgentID; }
  
    /** \brief Get the Time that this event was sent
        
        \return Send time of this event.
        \see Time
    */
    inline  Time getSentTime() const { return sentTime; }
  
    /** \brief Get the Time that this event will be delivered
        
        \return Delivery/receive time of this event
    */
    inline Time getReceiveTime() const { return receiveTime; }

    /** Convenience method to check reference count on events for
        troubleshooting/debugging purposes.

        \return The reference count on events. For valid events this
        value is in the range 0 < referenceCount < 3.
    */
    inline int getReferenceCount() const {return referenceCount; }

  
    /** \brief Determine if this Event is an anti-message
        
        \return True if this event is an anti-message.
    */
    inline bool isAntiMessage() const { return antiMessage; }

protected:
    /** \brief Type-setting constructor
        
        The constructor is protected to ensure that the user always
        uses the static CREATE method to create events. The AgentID of
        the agent receiving the event and the delivery time is the
        least amount of information needed to be a valid Event.
        
        \param receiverID the id of the receiver agent
        
        \param receiveTime this is the time the receiving agent should
        process this event.
        
        \see AgentID
        \see Time
    */
    explicit Event(const AgentID  receiverID, const Time  receiveTime);
    
    /** \brief Get the color of the Event
        
        This method must be used to determine the color value
        associated with this event.  The color value is typically used
        for GVT computations during simulation.
      
        \return The color value associated with this event.  The color
        value is always zero or one. Any other value potentially
        indicates an error.
    */
    inline char getColor() const { return color; }
  
    /** \brief Set the color value for this event.
        
        This method must be used to set the color value associated
        with this event.  The color value is typically used for GVT
        computations during simulation.
      
        \param color The color value associated with this event.
        The color value is always zero or one. Any other value
        potentially indicates an error.
    */
    void setColor(const char color);

    /** \brief Get the size of this Event
        
        This method is used to determine the correct size of each
        Event so that it can be properly sent across the wire by MPI.

        \note Users should override this method if event class is
        inherited and return correct size of the inherited event. For
        example the ClockEvent would override this and returns
        sizeof(ClockEvent)

        \return The size of the event
    */
    virtual int getEventSize() const { return sizeof(Event); }
  
    /** \brief Decrease the internal reference counter
        
        Used for memory management.  This method should not be used by
        users. MUSE uses this to handle memory for you.
    */
    void decreaseReference();
  
    /** \brief Increase the internal reference counter
        
        Used for memory management.  This method should not be used by
        users. MUSE uses this to handle memory for you.
    */
    void increaseReference();
  
    /** \brief Turn this Event into an anti-message
        
        Converts this event into an anti-message This method should
        only be used by MUSE internal operations.
    */
    void makeAntiMessage();

    /** \brief Directly set the internal reference counter

        Set the internal reference count to the specified value,
        bypassing the increase/decrease reference count methods.  Note
        that this method will not delete the message if the reference
        count is set to 0.

        This method is intended to be used only during rollbacks.
     */
    inline void setReferenceCount(int count) { referenceCount = count; }
 
    /** \brief Destructor
        
        Events are allocated by the CREATE method, and deleted by
        decreaseReference().  The Destructor will be called manually
        as the CREATE method allocates the Event in a special manner
        to guarantee that it is "flat" and ready to be sent across the
        wire.
    */
    virtual ~Event();

    /// The ID of the Agent that sent this Event
    AgentID senderAgentID;
    /// The ID of teh Agent that will receive this Event
    AgentID receiverAgentID;
    /// The Time this Event was sent
    Time sentTime;
    /// The Time this Event will be delivered/was received
    Time receiveTime;
    /// Is this an anti-message?
    bool antiMessage;

    /** Internal reference counter for memory management.

        The reference count for valid events being used for processing
        is either 1 or 2 (any other value would be temporary/invalid):

        <ol>

        <li>The reference count is 1 (particularly for remote events)
        indicating the event is in:

        <ul>

        <li>the scheduler's queue awaiting processing (or cancellation
        due to anti-messages)</li>

        <li>Agent's queue of processed events, maintained for
        rescheduling events after a rollback.</li>

        <li>Agent's output queue -- maintained for sending out
        anti-messages during a rollback.</li>

        </ul>

        </li>

        <li>The reference count is 2 for local events.  This is
        because the event is in the output queue of sending agent (on
        the same machine) as well as the input queue (scheduler or
        agent) of the receiving agent.</li>
        
        </ol>
    */
    int referenceCount; 
  
    /** \brief Instance variable to hold the color of this event for
        GVT computation.
      
        This instance variable is used to hold the color of this event
        for GVT computation using Mattern's GVT algorithm.  This
        character can take one of two values: 1 (for red events) and 2
        (for white events).  This information is set whenever an event
        is created and then used by the GVT manager to track events
        for GVT computation.
      
        \see GVTManager
    */
    char color;

    /** Convenience method to allocate flat memory for a given event.

        This method is a convenience method to streamline memory
        recycling operations for events.  This method allows memory
        recycling for events to be enabled/disabled using compile-time
        macro RECYCLE_EVENTS.

        <p>If recycling of events is disabled, this method always
        creates a flat array of characters using new[] operator and
        returns it.</p>

        <p>On the other hand, if recycling of events is enabled (via
        compiler flag RECYCLE_EVENTS), then this method operates as
        follows.  First it checks to see if EventRecycler has an entry
        of the specified size.  If so it returns it.  Otherwise it
        creates a new flat array of characters (using new[] operator)
        and returns it.</p>

        \param[in] size The size of the flat buffer to be allocated
        for storing event information.

        \return A pointer to a valid/flat buffer. This method always
        returns a valid event pointer.
    */
    static char* allocate(const int size);

    /** This method is the dual/converse of allocate to recycle events
        if recycling is enabled.

        This method is a convenience method to streamline memory
        recycling operations for events.  This method allows memory
        recycling for events to be enabled/disabled using compile-time
        macro RECYCLE_EVENTS.

        <p>If recycling of events is disabled, this method simply
        deletes the buffer using delete[] operator.</p>

        <p>On the other hand, if recycling of events is enabled (via
        compiler flag RECYCLE_EVENTS), then this method adds the
        buffer to the appropriate entry in the EventRecycler map in
        this class.</p>
        
        \param[in] buffer The event buffer previously obtained via
        call to the allocate method in this class.

        \param[in] size The size of the buffer (in bytes).  The size
        is used to recycle the buffer in future calls to allocate.
     */
    static void deallocate(char* buffer, const int size);
    
private:
    /** An unordered map of stacks to recycle events of different
        sizes.

        This map is used only if RECYCLE_EVENTS macro has been enabled
        at compile time.
        
        The key into this unordered map is the size of the event being
        recycled.  For each size a stack is maintained to return the
        most recently used event to improve cache performance.

        This map is used by the allocate and deallocate methods in
        this class.  Typically the Event::create method is the one
        that is used by applications to create an event.
    */
    static std::unordered_map<int, std::stack<char*>> EventRecycler;

    /** Helper method to clear out all events in the recycler.

        This method is typically called at the end of the simulation
        from Simulation::finalize() method.  This method deletes all
        events in the EventRecycler and empty's it.
    */
    static void deleteRecycledEvents();
};

END_NAMESPACE(muse);

#endif
