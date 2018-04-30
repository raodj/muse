#ifndef MUSE_STATE_H
#define MUSE_STATE_H

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
//          Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include "DataTypes.h"

BEGIN_NAMESPACE(muse);

/** The State class.

    This is a base class fo all states of Agents.  Any instance
    variable that changes as the simulation progresses must be stored
    in a corresponding State.  This base class allows MUSE to do
    automatic state management to make rollbacks transparent to the
    Agents.
*/
class State {
    friend class Agent;
    friend class OclAgent;
public:
    /** \brief Default Constructor.

        Initialize core variables shared across all States: the
        timestamp.
     */
    State();

    /** \brief Destructor

        Does nothing, as State does not allocate any memory.
    */
    virtual ~State();

    /** \brief Make a copy of this State
        
        This method must be overriden by all deriving classes.  This
        method should construct a new State (allocated on the heap)
        that is identical to this state.  This is typically
        accomplished with a copy constructor.
      
    */
    virtual State* getClone();
    
    /** \brief Get the time for which this State is valid
              
        \return Reference to the timestamp of this State
    */
    inline const Time& getTimeStamp() const { return timestamp; }

    /** Overloaded new operator for all states to streamline recycling
        of memory.

        This operator new is called whenever states are allocated.
        This method streamlines the process of allocating states while
        internally recycling memory for states.  The objective of
        memory recycling is to primarily reduce calls to the system's
        memory manager.  In addition, this memory manager is
        NUMA-aware.

        \param[in] sz The size of the state to be allocated.
     */
    void* operator new(size_t sz);

    /** Overloaded delete operator to enable recycling of states.

        This operator delete is called whenever states are garbage
        collected by the kernel.  This method recycles the memory
        block if state recycling is enabled (at compile time via
        RECYCLE_STATES) compiler flag.

        \param[in] state The state being deallocated.  This must be a
        pointer returned byc all to operator new in this class.  The
        pointer cannot be NULL.
    */
    void operator delete(void *state);
    
protected:
    /// The time at which this State represents
    Time timestamp;

private:
};

END_NAMESPACE(muse)
     
#endif
