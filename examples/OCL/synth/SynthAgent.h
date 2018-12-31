#ifndef OCL_SYNTH_AGENT_H
#define OCL_SYNTH_AGENT_H

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

#include "SynthAgentState.h"
#include "HCAgent.h"

/** A heterogeneous computing (HC) capable simulation agent (aka LP).

    This class provides a simple implementation for an agent capable
    of utilizing MUSE's heterogeneous computing (HC) capabilities.
    Specifically, standard event processing is done on the CPU while
    processing of epidemic equations is delegated to run on a GPU (if
    available).

    \note This class does not use the recommended standard MUSE macros
    to enable dynamic/variable number of compartments, parameters
    etc. for experimental analysis.  Instead it provides custom
    implementation for all the interface methods.
*/
class SynthAgent : public muse::HCAgent {
public:
    /** The only constructor for this class.

        The constructor initializes instance variables and the initial
        state for this agent.

        \param[in] id The simulation-wide unique ID set for this
        agent.  The IDs are assumed to be assigned in a row-major
        order.

        \param[in] x The logical x-coordinate in the grid of agents.

        \param[in] y The logical y-coordinate in the grid of agents.

        \param[in] solver Indicate if we need to run ODE (0), SSA (1),
        or SSA_TauLeap (2) versions.

        \param[in] step The time steps to be used for solving ODE or
        SSA systems. Eg. 0.01, 0.001 etc.
        
        \param[in] compartments The number of compartments in the
        model. This value must be positive.

        \param[in] paramCount The number parameters to emulate being
        used.
    */
    SynthAgent(muse::AgentID id, int x, int y, int solver,
               real step, int compartments, int paramCount);

    /** The destructor.
        
        The destructor does not any work to do as this class, per good
        coding practices, does not manage any dynamic memory.  The
        destructor is merely present to adhere to good coding
        conventions.
     */
    virtual ~SynthAgent() {}

    /** The initialization method.

        This method is invoked only once at the beginning of the
        simulation by MUSE kernel.  This method creates some initial
        events to itself to trigger epidemic processing.

        \note This method calls the runHCkernel method in muse::Agent
        to indicate that this agent requires HC support from MUSE.
    */
    virtual void initialize() override;

    /** Process events and simulate peform epidemic progression using
        HC support from MUSE.

        This method is called at different LVTs to process events at
        the given time.  This method processes incoming events and
        schedules an event to itself at the next time step to continue
        epidemic progressing.

        \note This method calls the runHCkernel method in muse::Agent
        to indicate that this agent requires HC processing at this LVT
        from MUSE.

        \param[in] events The set of events (all at the same LVT) to
        be processed by this method.
    */
    virtual void executeTask(const muse::EventContainer& events) override;

    /** Finalization method.

        This method is called after all the events have been processed
        and just before the simulation is terminated.  This method
        just prints the values in the compartments.
    */
    virtual void finalize() override;

    /** Conveienice method to setup the overall model dimensions.

        This static method is used by OclExampleSimulation to setup
        the total number of rows and columns in the model.

        \param[in] rows The total number of rows in the model.

        \param[in] cols The total number of columns in the model.

        \param[in] transition The number of transitions to be emulated
        if SSA version is run.

        \param[in] compartments The number of epidemic compartments
        associated with the transitions.  This should be the same
        value used to instantiate agents.

        \param[in] epiVals Flag to indicate if epidemic values should
        be printed by agents in the finalize method.  For big
        simulations just printing the final status can cause
        noticeable variation in timings.  So for performance
        comparisions this flag should be \c false.

        \param[in] eventsPerAgent The number of events to generate for
        each agent in each time step.

        \param[in] granularity The CPU load (approximately 1 is equal
        to 1 microsecond CPU time).
    */
    static void setParams(int rows, int cols, int transitions,
                          int compartments, bool showEpiVals,
                          int eventsPerAgent, int granularity);

protected:    
    /** Return the pre-support code back to the caller.

        This method is invoked to obtain any support code used in
        defining the state and parameters.  This method just returns
        the definition for the "real" data type.

        \return Support code/declarations used in defining
        state/parameters.
     */
    std::string getHCPreSupportCode() override;

    /** Return the general support code back to the caller.

        This method is invoked to obtain any support code used in
        conjunction with the kernel.  These are helper function etc.
        This function is defined in the source file using the
        HC_POST_SOURCE macro.

        \return Support code/declarations used in defining
        state/parameters.
     */
    std::string getHCPostSupportCode() override;

    /** Return the source code for the heterogeneous computing (HC)
        kernel code to be executed on a different device/GPU.

        This is a key interface method that a derived class must
        override to provide a custom definition for the source code to
        be used in the OpenCL kernel.  This method returns a custom
        OpenCL kernel for runnng on the device.

        \return The definition for the kernel source code to be
        executed on a different device/GPU to emulate ODE/SSA
        operations.
    */
    virtual std::string getHCkernelDefinition() override;

    /** Return the desired parameter definition to be reflected in the
        OpenCL kernel for this agent.  This is a convenience method
        that can a derived class can override to provide a custom
        definition for parameters to be used in the OpenCL kernel.
        This method is invoked only once (at the end of
        initialization) to generate the OpenCL kernel.

        \return The definition for streamlining parameter passing &
        usage on a GPU. This method returns a custom struct hc with
        a fixed number of additional parameters.
    */
    virtual std::string getHCParamDefinition() const override;

    /** Convenience method to run the HC kernel directly on the CPU.
        This method is used in situation where OpenCL is not being
        used.  In this case this method is immediately invoked after
        executeTask() to perform operations that would have normally
        run on a GPU.  This ensures that the same model can be reused
        on platforms that do not support OpenCL.
    */
    virtual void executeHCkernel();

    /** Helper method to generate transitions in the compChanges
        vector.

        This method is called from the setParams method.  This method
        populates the data in the compChanges vector by for each
        transitions.  Note that for each transition it generates
        changes for each compartment (essentially a matrix).  This
        results in "transitions * compartments number" of entries to
        be added to the compChanges vector.

        \param[in] transitions The number of transitions to be created.

        \param[in] compartments The number of compartments associated
        with each transitions.
     */
    static void generateSynthTransitions(const int transitions,
                                        const int compartments);

    /** Obtain the size (in bytes) for the parameters to be copied
        from the host to GPU for use on the GPU.  This method
        essentially returns the size of the synthetically generated
        structure:

        \code

        struct hc_params {
            int  solverType;
            real step;
            real params[PARAMETERS];
        };

        \endcode

        \note This method is frequently invoked and consequently must
        be designed to be fast.
        
        \return The size (in bytes) for the parameters to be copied
        from host to the GPU.
    */
    virtual int getHCParamSize() const override {
        // Each data in the generated synthetic structure is 8-byte
        // aligned (on 64-bit platforms)
        return (paramCount + 2) * 8;
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
    virtual void copyToDevice(void *dest, const int bufferSize) override;

    /** A simple loop that does trigonometry to place load on the CPU.

        \param[in] granularity The number of loops to
        execute. Approximatley each loop == 1 microsecond.
    */
    double simGranularity(int granularity) const;
    
private:
    /** The logical x-coordinate for this agent.  This value is set in
        the constructor and is never changed.  This value is in the
        range 0 <= x < cols.
    */
    int x;

    /** The logical y-coordinate for this agent.  This value is set in
        the constructor and is never changed.  This value is in the
        range 0 <= y < rows.
    */
    int y;
    
    /** Value to indicate if we need to run ODE (0), SSA (1), or
        SSA_TauLeap (2) versions.
    */
    int solverType;

    /** The step size to be used for ODE ans Tau-Leaping. */
    real step;
    
    /** The number of compartments to be emulated by this agent */
    int compartments;

    /** The number of parameters to be emulated by this synthetic
        agents.
    */
    int paramCount;

    /** The number of SSA transitions to be emulated by this synthetic
        agents.  By default the number of transitions is set to be
        2.5x the number compartments.  This value is also used to
        generate the compartment changes.  The transitions is based on
        the epidemic being simulated and consequently should be the
        same for all agents.  To ensure it is the same for all agents,
        it is defined to be static.  This value is set in the
        setParams method by SynthSimulation.
    */
    static int transitions;

    /** Matrix representing the changes in the different compartments
        caused by each state transition based on the values of
        different parameters.  The transitions is based on the
        epidemic being simulated and consequently should be the same
        for all agents.  To ensure it is the same for all agents, it
        is defined to be static.  The transition matrix/array is
        computed when the setParameters method in this class is
        invoked (just before simulation commences).
    */
    static std::vector<real> compChanges;
    
    /** The values for parameters to be copied to the GPU */
    std::vector<real> params;
    
    /** Total number of rows in the model. This value is shared by all
        the agents.  Consequently it is defined to be static.
    */
    static int rows;

    /** Total number of columns in the model. This value is shared by
        all the agents.  Consequently it is defined to be static.
    */
    static int cols;

    /** Flag to indicate if epidemic values should be printed by
        agents in the finalize method.  For big simulations just
        printing the final status can cause noticeable variation in
        timings.  So for performance comparisions the outputs should
        be turned off.
    */
    static bool showEpiVals;

    /** The number of events each agent should schedule to itself to
        emulate event processing */
    static int eventsPerAgent;

    /** Command-line arg to simulate some CPU load for each event */
    static int granularity;
    
    /** A pre-defined string that contains general helper code for
        solving ODE and SSA version of epidemic progression.  This
        string is returned from the getHCPreSupportCode method.
    */
    static const std::string EquationSolvers;

    /** A pre-defined string that contains the custom OCL kernel for
        the synthetic agent.  The custom kernel checks the solver type
        in the parameters and calls the corresponding ODE/SSA solver.
        This string is statically included in the executable by
        wrapping the SynthAgentKernel.c file as a raw C++11 string
        literal.  This string is returned from the
        getHCkernelDefinition method.
    */
    static const std::string SynthKernel;    
};

#endif
