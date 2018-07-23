#ifndef MUSE_HC_AGENT_H
#define MUSE_HC_AGENT_H

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

#include "Agent.h"
#include "HCState.h"
#include "MuseOclLibrary.h"

BEGIN_NAMESPACE(muse);

/** A simulation agent capable of heterogeneous compute operations.
    The operations of this agent are subdivided into two key steps.
    First, standard discrete event processing is done on the CPU.
    Next, agents that indicate additional processing on an OpenCL
    device are executed on the GPU.

    \note It is important to note that the two steps may not happen
    instantaneously and more importantly they do not happen on the
    same device/memory.  The necessary parameters and state
    information is copied to the device (typically GPU) via API
    methods in this class.

    In order to use this class a modeler needs to subdivide operations
    in the following manner:

    <ol>

    <li>The core discrete event processing continues to remain in the
    executeTask() method declared in muse::Agent. The derived model
    class must override the executeTask() method and perform necessary
    operations there.  This method is always executed on the CPU.

    \note If heterogeneous compute operations is required, then this
    method should call the runHCkernel method in the base class to let
    MUSE kernel know that this agent should be scheduled to also run
    on the GPU at the current LVT. </li>

    <li>

    \note In the derived agent class, it is best to use the HC_KERNEL
    macro to automatically generate the necessary implementation for
    these methods.
    
    The derived class must override hcKernel() in this class and also
    provide a suitable definition for the getHCkernelCode() method.
    In addition, the derived class must provide a suitable override
    for the runHCkernel() method.

    </li>

    <li>

    In addition, the derived agent class needs to structure any
    instance variables (or parameters) to be copied to the GPU in a
    suitable C structure for convenient/consistent operations.

    \note It is best to use the HC_PARAMETERS macro in MUSE to
    generate the necessary class parameter definitions and associated
    supporting methods.

    </li>
    
    </ol>
*/
class HCAgent : public muse::Agent {
    friend class muse::OclSimulation;
    friend class muse::Simulation;
public:
    /** The constructor.

        The constructor for Heterogeneous Computing (HC) capable
        agent.  The constructor for this class merely passes the
        parameters to the base class.

        \param[in] id The simulation-wide unique ID associated by the
        model to this agent.  This ID should be used to schedule
        events destined for this agent.

        \param[in] agentState The initial state to be set for this
        agent.  This pointer cannot be NULL.  The HCState class
        provides additional interface methods required for moving
        state information to-and-from devices/GPUs.
     */
    explicit HCAgent(muse::AgentID id, muse::HCState* agentState) :
        muse::Agent(id, agentState) {}

    /** The destructor.

        The destructor for this class.  The destructor does not have
        any special operations.
     */
    virtual ~HCAgent() {}

    /** Overrides method that dervied classes to use to indicate that
        heterogeneous compute kernel associated with this agent must
        be executed at this LVT.  

        This method intercepts this call and execute the HC kernel
        right away if the simulation is run using a simulation-kernel
        that does not support heterogeneous computing.
        
        \note This method must be called prior to executeTask method
        ends.  For example:

        \code

        EpiAgent::executeTask(const EventContainer& eventList) {
            // Use a loop to process events in eventList

            // Now tell MUSE that it should run the heterogeneous
            kernel for this agent at this LVT
            runHCkernel(0);
        }

        \endcode

        \param[in] kerenelID The logical ID associated with the OpenCL
        kernel that must be executed.  Currently, this parameter is
        not really used.
    */
    virtual void runHCkernel(const int kernelID = 0) override;
    
protected:
    /** Return the desired parameter definition to be reflected in the
        OpenCL kernel for this agent.  This is a convenience method
        that can a derived class can override to provide a custom
        definition for parameters to be used in the OpenCL kernel.
        This method is invoked only once (at the end of
        initialization) to generate the OpenCL kernel.

        \note Typically (uncless you have complex parameters) this
        method is atuomatically generated in a derived class using the
        HC_PARAMETERS macro.

        \return The definition for streamlining parameter passing &
        usage on a GPU. An example return value would be:

        \code

        // The struct must be named hc for compatibility with
        // rest of the system.
        return "struct hc { int i; double d; }";

        \endcode.
     */
    virtual std::string getHCParamDefinition() const {
        return "struct hc {}";
    }
    
    /** Obtain the size (in bytes) for the parameters to be copied
        from the host to GPU for use on the GPU.  This method
        essentially returns the size of the resulting structure/code
        returned by the getHCParamDefinition() method.  For example,
        if the structure returned by getHCParamDefinition method is
        "struct hc { int i; double d; }" then this method should
        return 16.

        \note This method is frequently invoked and consequently must
        be designed to be fast.

        \note Typically (unless you have complex parameters) this
        method is automatically generated in a derived class using the
        HC_PARAMETERS macro.
        
        \return The size (in bytes) for the parameters to be copied
        from host to the GPU.  The default implementation returns
        zero.
    */
    virtual int getHCParamSize() const {
        return 0;
    }

    /** Convenience method to copy the parameters to the device's
        buffer.  This method is invoked each time parameters need to
        be copied to the device/GPU.

        \param[in,out] dest The destination location where the parameter
        data is to be copied.  This pointer is never null.

        \param[in] bufferSize Variable that indicates the buffer space
        actually reserved.  When things are working correctly, this
        value should be the same as the value returned by
        getHCParamSize() method.
    */
    virtual void copyToDevice(void *dest, const int bufferSize) {
        UNUSED_PARAM(dest);
        UNUSED_PARAM(bufferSize);
    }

    /** Interface method to return any supporting code/functions for
        direct/indirect use by OpenCL kernel.

        This method is invoked once to get any supporting code used in
        the heterogeneous computing (HC) code path.  The support code
        should be in C language.  This method is invoked before the
        core state and parameter definitions.  Consequently, this
        method should provide key type definitions used for defining
        parameters and state data.

        \note It is best ot use the HC_PRE_SOURCE macro to generate the
        definition of this method.

        \return Any additional supporting code used in the OpenCL
        kernel.
    */
    virtual std::string getHCPreSupportCode() {
        return "";
    }

    /** Interface method to return any supporting code/functions for
        direct/indirect use by OpenCL kernel.

        This method is invoked once to get any supporting code used in
        the heterogeneous computing (HC) code path.  The support code
        should be in C language.  This method is invoked after the
        core state and parameter definitions.  Consequently, this
        method should be used to implement helper functions etc. that
        use the state/parameter definitions as a whole.

        \note It is best ot use the HC_POST_SOURCE macro to generate
        the definition of this method.

        \return Any additional supporting code used in the OpenCL
        kernel.
    */
    virtual std::string getHCPostSupportCode() {
        return "";
    }
    
    /** Return the source code for the heterogeneous computing (HC)
        kernel code to be executed on a different device/GPU.

        This is a key interface method that a derived class can
        override to provide a custom definition for the source code to
        be used in the OpenCL kernel.  This method is invoked only
        once (at the end of initialization) to generate the OpenCL
        kernel.

        \note Typically (uncless you have complex parameters) this
        method is atuomatically generated in a derived class using the
        HC_KERNEL macro in the source file for an agent.

        \return The definition for the kernel source code to be for
        executed on a different device/GPU.  An example return value
        would be:

        \note The signature of the kernel function (hcKernel) should
        be exactly as shown below to ensure correct operation.
        
        \code

        // The parameters and state information is supplied as
        // pointers named hc_params and hc_state for use in this
        // method.

        return R"(
            void hcKernel(int id, __constant hc_params* hc_params,
                          __local hc_state* hc_state) {
               hc_state->sus -= hc_state->inf * hc_params->beta;"
               hc_state->inf += hc_state->sus * hc_params->beta;
         )";
        \endcode.
     */
    virtual std::string getHCkernelDefinition() {
        return "";
    }
    
    /** Convenience method to run the HC kernel directly on the CPU.
        This method is used in situation where OpenCL is not being
        used.  In this case this method is immediately invoked after
        executeTask() to perform operations that would have normally
        run on a GPU.  This ensures that the same model can be reused
        on platforms that do not support OpenCL.

        \note Typically, this method is automatically generated using
        HC_KERNEL macro in muse.
    */
    virtual void executeHCkernel() {}

    /** The shared random number generator interface.  This pointer is
        initialized with a shared random number generator data.  This
        structure is used on the CPU only (as fallback when OpenCL is
        not used).  It is defined to be a pointer to avoid polluting
        the interface by havin to include actual definitions.  This
        pointer should be used as a opaque pointer.
    */
    static struct MTrand_Info rndInfo;
};

END_NAMESPACE(muse);

#endif
