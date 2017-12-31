#ifndef MUSE_EVENT_ADAPTER_H
#define MUSE_EVENT_ADAPTER_H

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
// Authors:  Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "Event.h"

BEGIN_NAMESPACE(muse);

/** Convenience class to streamline access to private instance
    variables in an Event.

    This class is designed to provide a consistent interface to access
    the private instance variables in an event.  These instance
    variables are meant to be accessed only by classes in the kernel.
    This class essentially centralizes access to the private instnace
    variables.

    \note This class contains only static methods that are meant to be
    called directly.  This class is not meant to be instantiated.
*/
class EventAdapter {
    friend class GVTManager;
    friend class Agent;
    friend class MultiThreadedSimulation;
public:

    /** \brief Helper to get the size of this Event
        
        This method is used to determine the correct size of each
        Event so that it can be properly sent across the wire by MPI.
        This method provides the kernel with direct access to
        determine the event size.

        \param[in] event The event whose size is to be determined.
        This parameter cannot be NULL/nullptr.
        
        \return The size of the event

        \see muse::Event::getEventSize
    */
    static inline int getEventSize(const muse::Event* const event) {
        return event->getEventSize();
    }
    
protected:
    /** \brief Get the color of the Event
        
        This method must be used to determine the color value
        associated with this event.  The color value is typically used
        for GVT computations during simulation.

        \param[in] event The event whose color is to be returned.
        This pointer cannot be NULL.

        
        \return The color value associated with this event.  The color
        value is always zero or one. Any other value potentially
        indicates an error.
    */
    static inline char getColor(const muse::Event* const event) {
        return event->color;
    }
  
    /** \brief Set the color value for this event.
        
        This method must be used to set the color value associated
        with this event.  The color value is typically used for GVT
        computations during simulation.

        \param[in,out] event The event whose color is to be changed
        based on the GVT cycle.
        
        \param color The color value associated with this event.
        The color value is always zero or one. Any other value
        potentially indicates an error.
    */
    static inline void setColor(muse::Event* const event, const char color) {
        event->color = color;
    }

    /** \brief Turn this Event into an anti-message

        \param[in,out] event The event to be converted to an
        anti-message.  This pointer cannot be NULL.
        
        Converts this event into an anti-message This method should
        only be used by MUSE internal operations.
    */
    static inline void makeAntiMessage(muse::Event* const event) {
        event->antiMessage = true;
    }

    /** \brief Directly set the internal reference counter

        Set the internal reference count to the specified value,
        bypassing the increase/decrease reference count methods.  Note
        that this method will not delete the message if the reference
        count is set to 0.

        This method is intended to be used only during rollbacks.

        \param[in,out] event The event whose reference count is to be
        set.

        \param[in] count The reference counter value to be set.
     */
    static inline void setReferenceCount(muse::Event* const event, int count) {
        event->referenceCount = count;
    }

    /** \brief Helper to set the time when the event was sent.
        
        This method is used to set the time when an event was sent.
        This value is typically set in the Agent::scheduleEvent
        method. This method provides the kernel with direct access to
        set the sent time for the event.

        \param[in,out] event The event whose sent time is to be set by
        this method.

        \param[in] sentTime The virtual time when this event was
        logically sent.
    */
    static inline void setSentTime(muse::Event* const event,
                                   const muse::Time sentTime) {
        event->sentTime = sentTime;
    }

    /** \brief Helper to set the agent that is originating/sending an
        event.
        
        This method is used to set the ID of the agent that is
        originating/sending an event.  This value is typically set in
        the Agent::scheduleEvent method. This method provides the
        kernel with direct access to set the senderAgentID for the
        event.

        \param[in,out] event The event whose sent time is to be set by
        this method.

        \param[in] sender The MUSE ID of the agent that is sending
        this event.
    */
    static inline void setSenderAgentID(muse::Event* const event,
                                        const muse::AgentID sender) {
        event->senderAgentID = sender;
    }

    /** \brief Helper to set the agent that is originating/sending an
        event and the sent time.

        This method is a convenience method to setup the sender ID and
        sent time values of an event in one method call. This method
        is used to set the ID of the agent that is originating/sending
        an event along with timestamp.  This value is typically set in
        the Agent::scheduleEvent method. This method provides the
        kernel with direct access to set the senderAgentID for the
        event.

        \param[in,out] event The event whose sent time is to be set by
        this method.

        \param[in] sender The MUSE ID of the agent that is sending
        this event.

        \param[in] sentTime The virtual time when this event is being
        scheduled.
    */
    static inline void setSenderInfo(muse::Event* const event,
                                     const muse::AgentID sender,
                                     const muse::Time sentTime) {
        event->senderAgentID = sender;
        event->sentTime      = sentTime;
    }
    
private:
    /** The only constructor that is intentionally private and
        undefined to ensure that this class is never instantiated.
    */
    EventAdapter();

    /** The destructor that is intentionally private and undefined to
        ensure that this class is never instantiated.
    */
    ~EventAdapter();
};

END_NAMESPACE(muse);

#endif
