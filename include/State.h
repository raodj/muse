#ifndef _MUSE_STATE_H_
#define _MUSE_STATE_H_

#include "DataTypes.h"
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
//
//---------------------------------------------------------------------------


BEGIN_NAMESPACE(muse) //begin namespace declaration 

/** The State class.
    This class is a base class and SHOULD be used as a superclass. All agents must have a State. 
    Think of the State as all the information that can be changed within an agent. When an agent
    has to process a collection of events, what actually happens is information in the state is changed
    based on the event being processed. 
 */
class State {

  //let make the Agent class a friend
  friend class Agent;
  
 public:
  /** The getClone method.
      This method must be implemented by the client. This is because there is no way 
      muse can know what information the client has in the State class. 
      
      @return State* a pointer to the clone of this State. Should be a new state allocated in the heap.
  */
  virtual State * getClone(); //must override this.
  
  /** The ctor method.
  */
  State();

  /** The getTimeStamp method.
      @return reference to the timestamp of this state.
   */
  inline const Time & getTimeStamp() const { return timestamp;}
  
 
  /** The dtor method.
      Cleans up all mess created by muse.
  */
  virtual ~State();
  
 protected:
  // this is used for storage purpose. Incase of a rollback this is used to get a state before the rollback time. 
  Time timestamp;
};

END_NAMESPACE(muse)
     
#endif
