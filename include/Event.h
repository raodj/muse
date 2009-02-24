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
  friend class GVTManager;
 public:
  /** The ctor method.
      This is the fastest way to create an Event object. The AgentID
      of the agent receiving the event and the delivery time is the
      least amount of information needed to be a vaild Event.
      
     
      @param receiverID the id of the receiver agent
      @param receiveTime this is the time the receiving agent should process this event.

      @see AgentID
      @see Time
  */
  explicit Event(const AgentID  receiverID,const Time  receiveTime);

  /** The getSenderAgentID method.
      @return reference to the sender AgentID.
      @see AgentID
  */
  inline const AgentID& getSenderAgentID() const { return senderAgentID;}

  /** The getReceiverAgentID method.
      @return reference to the receiver AgentID.
      @see AgentID
  */
  inline const AgentID& getReceiverAgentID() const { return receiverAgentID; }
  
  /** The getSentTime method.
      @return reference to the sent time of this event.
      @see Time
  */
  inline const Time& getSentTime() const { return sentTime; }
  
  /** The getReceiveTime method.
      @return reference to the receive time of this event.
  */
  inline const Time& getReceiveTime() const { return receiveTime; }

 protected:
  /** The getColor method.
      This method must be used to determine the color value
      associated with this event.  The color value is typically used
      for GVT computations during simulation.
      
      @return The color value associated with this event.  The color
      value is always zero or one. Any other value potentially
      indicates an error.
  */
  inline char getColor() const { return color; }
  
  /** Set the color value for this event.
      This method must be used to set the color value associated
      with this event.  The color value is typically used for GVT
      computations during simulation.
      
      @param color The color value associated with this event.
      The color value is always zero or one. Any other value
      potentially indicates an error.
  */
  void setColor(const char color);


  /** The getEventSize method.
      This method is very important, when it comes time to send an event
      across the wire via MPI.

      @note USERS should override this method if event class is inherited and return
      correct size of the inherited event. For example the ClockEvent would override
      this and returns sizeof(ClockEvent)

      @return int, the size of the event.
   */
  virtual int getEventSize() {return sizeof(Event); }
  
  /** The decreaseReference method.
      Used for memory management.
      This method should not be used by users. MUSE usues this to handle memory for you.
  */
  void decreaseReference();
  
  /** The increaseReference method.
      Used for memory management.
      This method should not be used by users. MUSE usues this to handle memory for you.
  */
  void increaseReference();
  
  /** The isAntiMessage method.
      @return bool True if this event is an anti-message.
  */
  inline bool isAntiMessage() const { return antiMessage; }
  
  /** The makeAntiMessage
      converts this event into an anti-message
      This method should only be used by muse internal operations.
  */
  void makeAntiMessage();

 

 
  /** The dtor.
      User should not be able to delete events. Also events can only be created
      in the heap.
   */
  ~Event(); 
  
  AgentID senderAgentID, receiverAgentID;
  Time sentTime,receiveTime;
  //this is used to let the scheduler know that this event is an anti-message
  bool antiMessage;      
  //this is for memory managements
  int referenceCount; 
  
  /** Instance variable to hold the color of this event for GVT
      computation.
      
      This instance variable is used to hold the color of this event
      for GVT computation using Mattern's GVT algorithm.  This
      character can take one of two values: 1 (for red events) and 2
      (for white events).  This information is set whenever an event
      is created and then used by the GVT manager to track events
      for GVT computation.
      
      @see GVTManager
  */
  char color;

  /** The following two methods are used by boost::intrusive_ptr
      for low memory footprint smart shared pointers. Makes life
      so much easier. 
   */
  friend void intrusive_ptr_add_ref(Event * p);
  friend void intrusive_ptr_release(Event * p);
  
  
};


inline void intrusive_ptr_add_ref(Event * p)
{
    // increment reference count of object *p
    std::cout << "increasing ref"<<std::endl;
    p->increaseReference();
}

inline void intrusive_ptr_release(Event * p)
{
    // decrement reference count, and delete object when reference count reaches 0
    std::cout << "decreasing ref"<<std::endl;
    p->decreaseReference();
}


END_NAMESPACE(muse); //end namespace declaration

/** \def INFINITY
    
\brief A #define for virtual time corresponding to infinity.

This define provides a more human readable time corresponding to
infinity that is used for gvt computations.  This is also the
default value to which time instance variables are initialized.
*/
#define INFINITY Time(1e30)

#endif
