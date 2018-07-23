#ifndef MUSE_HC_STATE_H
#define MUSE_HC_STATE_H

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
// Authors: Dhananjai M. Rao    raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include "HCMacros.h"
#include "State.h"

BEGIN_NAMESPACE(muse);

/** An extended state class for Heterogeneous Computing (HC)
    operations.

    This is an extended state class used by HCAgent for holding state
    information which is modified by CPU (during event processing) and
    by a device/GPU (when HC kernel is executed).  This class
    essentially serves as a place holder for various API methods used
    by MUSE for enabling HC simulations.

    \note Derived classes can use the HC_STATE macro to generate
    custom implementation for the interface methods in this class.
*/
class HCState : public muse::State {
    friend class Agent;
    friend class OclAgent;
public:
    /** \brief Default Constructor.

        This is just a placeholder constructor.  This class currently
        does not have any instance variables for the constructor to
        initialize.
    */
    HCState() {}
    
    /** \brief Destructor

        Does nothing, as this class does not allocate any memory.
    */
    virtual ~HCState() {}

    /** Interface method to return the state definition to be
        reflected in the HC kernel code.  This method is invoked only
        once after initialization to construct the source code for the
        kernel.

        \note In a derived class, it is best to use the HC_STATE macro
        to generate the implementation for this method.

        \return The source code for the state variables to be
        reflected on a different device/GPU.  An example return value
        would be:

        \code

        return "struct hc { int sus; int inf; }";

        \endcode
     */
    virtual std::string getHCStateDefinition() const {
        return "";
    }

    /** Return the size (in bytes) of the state variables to be
        copied/retrieved from a different device/GPU.

        \note Typically (unless you have complex state) this
        method is automatically generated in a derived class using the
        HC_STATE macro.

        \return The size (in bytes) for the state information to be
        copied from host to the GPU.  For example, if the structure
        returned by getHCStateDefinition() is "struct hc { int i;
        double d; }", then this method should return just 16 (and not
        the full size of the state).
    */
    virtual int getHCStateSize() const {
        return 0;
    }

    /** Convenience method to copy subset of the state to the device's
        buffer.  This method is invoked each time state information
        need to be copied to the device/GPU (so it should be fast).

        \note Typically (unless you have complex state) this
        method is automatically generated in a derived class using the
        HC_STATE macro.
        
        \param[in,out] dest The destination location where the state
        data is to be copied.  This pointer is never null.

        \param[in] bufferSize Variable that indicates the buffer space
        actually reserved.  When things are working correctly, this
        value should be the same as the value returned by
        getHCStateSize() method.
    */
    virtual void copyToDevice(void *dest, const int bufferSize) {
        UNUSED_PARAM(dest);
        UNUSED_PARAM(bufferSize);
    }

    /** Convenience method to copy subset of the state from a
        device/GPU's buffer.  This method is invoked each time state
        information need to be copied back from the device/GPU (so it
        should be fast).

        \note Typically (unless you have complex state) this
        method is automatically generated in a derived class using the
        HC_STATE macro.
        
        \param[in] src The source location from where the state
        data is to be copied.  This pointer is never null.

        \param[in] bufferSize Variable that indicates the buffer space
        associated with src.  When things are working correctly, this
        value should be the same as the value returned by
        getHCStateSize() method.
    */
    virtual void copyFromDevice(const void *src, const int bufferSize) {
        UNUSED_PARAM(src);
        UNUSED_PARAM(bufferSize);        
    }
};


END_NAMESPACE(muse);

#endif
