R"(
#ifndef %CLASS_NAME%_H
#define %CLASS_NAME%_H

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

// Ensure that macros is included first.
#include "HCState.h"

// For now we fix the number of epidemic compartments at compile time.
// This can be changed by specifying a -DCOMPARTMENTS=5 compiler flag.
#ifndef COMPARTMENTS
#define COMPARTMENTS %COMPS%
#endif

/** \typedef real

    \brief Convenience typedef to ease using float or double

    This typedef eases changing between float vs. double to
    accommodate running on GPGPUs that may not support double.
*/
using real = double;

/** A simple Heterogeneous Computing (HC) state class to hold
    epidemic-comparatement information.

    This state class uses the set of macros provided by MUSE (in
    HCMacros.h) to generate HC capable state.  Specifically,
    compartments in the state are encapsulated into an <tt>hc</tt>
    sub-container so that they can be easily moved back-and-forth from
    a GPU.
*/
class %CLASS_NAME% : public muse::HCState {
    friend class OclExampleAgent;
public:
    /** Default constructor.

        The constructor merely initializes all the instance variables
        to zeros.
    */
    %CLASS_NAME%() {
        // The agent will update compartments when the agent is
        // instantiated.  Note the sub-container hc which is used to
        // mirror the data on GPUs
        for (int i = 0; (i < %COMPS%); i++) {
            hc.comps[i] = 0;
        }
    }

    /** Copy constructor.

        Initializes instance variables using information from the
        supplied source state.

        \note This method correctly copies the timestamp value in the
        state.
        
        \param[in] src The source object from where the data is to be
        copied.

        \see getClone
    */
    %CLASS_NAME%(const %CLASS_NAME%& src) : hc(src.hc) {
        // The copy constructor correctly initializes instance variables
        // and does not use assignment operator to assign value.
        
        // NOTE: In the above initialization, hc is a Plain-Old-Data (POD)
        // data type and consequently we get away without having to
        // initialize each individual value.
    }

    /** The destructor.

        The destructor does not any work to do as this class, per good
        coding practices, does not manage any dynamic memory.  The
        destructor is merely present to adhere to good coding
        conventions.
    */
    virtual ~%CLASS_NAME%() {}
    
    /** \brief Make a copy of this State for state saving used with
        optimistic PDES.
        
        As per MUSE API, this method must be overriden by all deriving
        classes.  This method should construct a new State (allocated
        on the heap) that is identical to this state.  This is
        typically accomplished with a copy constructor.
    */
    virtual muse::State* getClone() override {
        // Use copy constructor to make a copy.
        return new %CLASS_NAME%(*this);
    }

    /** A HC state that contains the population of individuals in
        various epidemic compartments of a standard SEIR model.  The
        HC_STATE macro is provided by MUSE.
    */
    HC_STATE(
         real comps[COMPARTMENTS];
    );
};

#endif
)"
