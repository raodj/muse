#ifndef SYNTH_AGENT_STATE_H
#define SYNTH_AGENT_STATE_H

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
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "HCState.h"
#include <algorithm>

/** \typedef real

    \brief Convenience typedef to ease using float or double

    This typedef eases changing between float vs. double to
    accommodate running on GPGPUs that may not support double.
*/
using real = double;

/** A simple Heterogeneous Computing (HC) state class to hold
    epidemic-comparatement information.

    Unlike typical applications, this state class intentionally does
    not uses the set of macros provided by MUSE (in HCMacros.h) to
    generate HC capable state.  Instead it uses a variable-sized state
    for experimentation and provides custom implementation for
    interface methods to handle variable-sized state.

    \note The state size for OpenCL is fixed when the kernel is
    intially generated.
*/
class SynthAgentState : public muse::HCState {
    friend class SynthAgent;
public:
    /** Default constructor.

        The constructor merely initializes all the instance variables
        to zeros.

        \param[in] compartments The number of compartments in the
        state.  The default is 4.
    */
    SynthAgentState(const int compartments = 4);

    /** Copy constructor.

        Initializes instance variables using information from the
        supplied source state.

        \note This method correctly copies the timestamp value in the
        state.
        
        \param[in] src The source object from where the data is to be
        copied.

        \see getClone
    */
    SynthAgentState(const SynthAgentState& src);

    /** The destructor.

        The destructor does not any work to do as this class, per good
        coding practices, does not manage any dynamic memory.  The
        destructor is merely present to adhere to good coding
        conventions.
    */
    virtual ~SynthAgentState() {}
    
    /** \brief Make a copy of this State for state saving used with
        optimistic PDES.
        
        As per MUSE API, this method must be overriden by all deriving
        classes.  This method should construct a new State (allocated
        on the heap) that is identical to this state.  This is
        typically accomplished with a copy constructor.
    */
    virtual muse::State* getClone() override {
        // Use copy constructor to make a copy.
        return new SynthAgentState(*this);
    }

    /** Interface method to return the state definition to be
        reflected in the HC kernel code.  This method is invoked only
        once after initialization to construct the source code for the
        kernel.

        \return This method returns a a structure containing a fixed
        number of compartments.  The number of compartments is
        determined by the size of the comps vector.
    */
    virtual std::string getHCStateDefinition() const;

    /** Return the size (in bytes) of the state variables to be
        copied/retrieved from a different device/GPU.

        \return The size (in bytes) based on the number of
        compartments.
    */
    virtual int getHCStateSize() const {
        return sizeof(real) * comps.size();
    }

    /** Convenience method to copy subset of the state from a
        device/GPU's buffer.  This method is invoked each time state
        information need to be copied back from the device/GPU (so it
        should be fast).

        \param[in] src The source location from where the state
        data is to be copied.  This pointer is never null.

        \param[in] bufferSize Variable that indicates the buffer space
        associated with src.  When things are working correctly, this
        value should be the same as the value returned by
        getHCStateSize() method.
    */
    virtual void copyFromDevice(const void *src, const int bufferSize) {
        ASSERT(src != NULL );
        ASSERT(bufferSize == getHCStateSize());
        UNUSED_PARAM(bufferSize);
        // Copy the data into the comps vectro in the state
        const real* srcData = reinterpret_cast<const real*>(src);
        std::copy_n(srcData, comps.size(), comps.begin());
    }

    /** Convenience method to copy subset of the state to the device's
        buffer.  This method is invoked each time state information
        need to be copied to the device/GPU (so it should be fast).
        
        \param[in,out] dest The destination location where the state
        data is to be copied.  This pointer is never null.

        \param[in] bufferSize Variable that indicates the buffer space
        actually reserved.  When things are working correctly, this
        value should be the same as the value returned by
        getHCStateSize() method.
    */
    virtual void copyToDevice(void *dest, const int bufferSize) {
        ASSERT(dest != NULL);
        ASSERT(bufferSize == getHCStateSize());
        UNUSED_PARAM(bufferSize);
        // Copy the data from comps into the buffer
        real* destData = reinterpret_cast<real*>(dest);
        std::copy_n(comps.begin(), comps.size(), destData);
    }

protected:
    /** The variable length synthetic compartments in this model. The
        number of compartments is initially set by the agent.
        Subsequently, all the states will be of the same size.
    */
    std::vector<real> comps;
};

#endif
