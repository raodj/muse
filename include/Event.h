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
    friend class EventRecycler;
    friend class EventAdapter;
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
    
    /** \brief A templatized convenience method for creating an event
        derived from  muse::Event base class.
        
        This is a convenience method that can be used to create an event
        object with a given set of arguments passed to the constructor.
        This method can be used in the following manner:
        
        \code
        
        // ... some code ...
        muse::Event* event = muse::create<Event>(receiverID, recvTime);
        // ... more code ...
        scheduleEvent(event);
        
        \endcode
        
        @tparam T the type of derived event class to be created.
        
        @Args A variadic list of zero or more arguments to be passed to
        the constructor of the event.  Note that the event must implement
        the constructor in the order in which arguments are supplied.
    */
    template<typename T, typename... Args> 
    static T* create( Args&&... args ) {
        T* event = reinterpret_cast<T*>(allocate(sizeof(T)));
        new (event) T(std::forward<Args>(args)...);
        return event;
    }

    /** \brief A templatized convenience method for creating an event
        with a given size.
        
        This is a convenience method that can be used to create an
        event object of given size with a given set of arguments
        passed to the constructor.  This method can be used in the
        following manner:
        
        \code
        
        // ... some code ...
        muse::Event* const event =
           muse::create<Event, 0>(bytes, receiverID, recvTime);
        // ... more code ...
        scheduleEvent(event);
        
        \endcode
        
        @tparam T the type of derived event class to be created.

        @tparam dummy A dummy parameter that is ignored but must be
        specified (typically as literal constant 0).  It is merely
        used to distinguish the different overload of the static
        create method in this class.
        
        @Args A variadic list of zero or more arguments to be passed to
        the constructor of the event.  Note that the event must implement
        the constructor in the order in which arguments are supplied.
    */
    template<typename T, size_t dummy = 0, typename... Args> 
    static T* create(size_t size, Args&&... args) {
        T* event = reinterpret_cast<T*>(allocate(size));
        new (event) T(std::forward<Args>(args)...);
        return event;
    }

    /** \brief A templatized convenience method for creating
        clone/deep-copy of this event.
        
        This is a convenience method that can be used to create a
        clone/copy of this event.  This method internall uses the
        copy-constructor to create a copy.  Consequently, the derived
        event class must provide a suitable public copy-constructor.
        
        \code
        
        // ... some code ...
        muse::Event* event;
        muse::Event* copy = event->create<muse::Event>();
        // ... more code ...
        scheduleEvent(event);
        
        \endcode
        
        @tparam T the type of derived event class to be created.  This
        class must provide a public copy constructor of the following
        form for this method to use:

        \code

        T::T(const T& src) : muse::Event(src.getReceiverAgentID(),
                                         src.getReceiveTime()) {
            // Implement code to copy each and every instance member...
        }

        \endcode

        @param[in] src The source event object that must be cloned.
        We accept a reference to ensure that the source is not
        nullptr.  The size of the source object is used to determine
        the amount of memory to be allocated.
    */
    template<typename T> 
    T* create() const {
        T* event = reinterpret_cast<T*>(allocate(this->getEventSize()));
        new (event) T(dynamic_cast<const T&>(*this));
        return event;
    }
    
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
    
protected:    
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
    
    /** \brief Destructor
        
        Events are allocated by the CREATE method, and deleted by
        decreaseReference().  The Destructor will be called manually
        as the CREATE method allocates the Event in a special manner
        to guarantee that it is "flat" and ready to be sent across the
        wire.
    */
    virtual ~Event() {}

    /// The ID of teh Agent that will receive this Event
    AgentID receiverAgentID;
    /// The Time this Event will be delivered/was received
    Time receiveTime;

private:
    /// The ID of the Agent that sent this Event
    AgentID senderAgentID;
    
    /// The Time this Event was sent
    Time sentTime;
    
    /** Flag to indicate if this event is an anti-message.  If the
        flag is true, then the event is an anti-message.  The default
        value for this flag is false.
    */
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
    char referenceCount; 
  
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

    /** \brief Internal reference counter for memory management in
        multi-threaded scenarios.

        This reference counter is used to track presence of events in
        input queues of agents and schedulers.  This reference counter
        is only updated on receivers of events that have events in
        scheduler or Agent's-input-queues.  Using 2 independent
        reference counters enables sender and receiver threads to
        independently update them and then delete/recycle events when
        both counters become zero.  By default this counter is
        initialized to zero.

        \note In single-threaded mode this reference counter is not
        used.
    */
    char inputRefCount;
};

END_NAMESPACE(muse);

#endif
