R"(
#ifndef %CLASS_NAME%_AGENT_H
#define %CLASS_NAME%_AGENT_H

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

#include "%CLASS_NAME%State.h"
#include "HCAgent.h"

// For now we fix the number of epidemic parameters at compile time.
// This can be changed by specifying a -DCOMPARTMENTS=5 compiler flag.
#ifndef PARAMETERS
#define PARAMETERS %PARAMS%
#endif

/** A heterogeneous computing (HC) capable simulation agent (aka LP).

    This class provides a simple implementation for an agent capable
    of utilizing MUSE's heterogeneous computing (HC) capabilities.
    Specifically, standard event processing is done on the CPU while
    processing of epidemic equations is delegated to run on a GPU (if
    available).  The classes uses various standard macros provided by
    MUSE to generate code to be moved/deployed to the GPU.
*/
class %CLASS_NAME%Agent : public muse::HCAgent {
public:
    /** The only constructor for this class.

        The constructor initializes instance variables and the initial
        state for this agent.

        \param[in] id The simulation-wide unique ID set for this
        agent.  The IDs are assumed to be assigned in a row-major
        order.

        \param[in] x The logical x-coordinate in the grid of agents.

        \param[in] y The logical y-coordinate in the grid of agents.

        \param[in] ode If this flag is true then ODE version is run by
        this agent.  Otherwise it runs SSA version.

        \param[in] step The time steps to be used for solving ODE or
        SSA systems. Eg. 0.01, 0.001 etc.
        
        \param[in] compVals The initial value to be used for the
        compartments.
    */
    %CLASS_NAME%Agent(muse::AgentID id, int x, int y, bool ode,
                      real step, const std::vector<real>& compVals);

    /** The destructor.
        
        The destructor does not any work to do as this class, per good
        coding practices, does not manage any dynamic memory.  The
        destructor is merely present to adhere to good coding
        conventions.
     */
    virtual ~%CLASS_NAME%Agent() {}

    /** The initialization method.

        This method is invoked only once at the beginning of the
        simulation by MUSE kernel.  This method creates some initial
        events to itself to trigger epidemic processing.

        \note This method calls the runHCkernel method in muse::Agent
        to indicate that this agent requires HC support from MUSE.
    */
    virtual void initialize() throw (std::exception) override;

    /** Process events and peform epidemic progression using HC
        support from MUSE.

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
    */
    static void setParams(int rows, int cols);
    

    /* Use MUSE macros to define parameters to be copied to the GPU */
    HC_PARAMETERS(
                  bool  ode;  /* ODE vs. SSA flag */
                  real  step; /* The step size to use */
                  real params[PARAMETERS];
                  );

protected:    
    /** Declare the main entry point and interface methods for
        heterogeneous computing (HC) kernel.

        Here we just use MUSE's helper macros to declare the various
        methods.
    */
    HC_KERNEL_DECL(%CLASS_NAME%Agent, %CLASS_NAME%State);

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
    
    /** Total number of rows in the model. This value is shared by all
        the agents.  Consequently it is defined to be static.
    */
    static int rows;

    /** Total number of columns in the model. This value is shared by
        all the agents.  Consequently it is defined to be static.
    */
    static int cols;

    /** A pre-defined string that contains general helper code for
        solving ODE and SSA version of epidemic progression.  This
        string is returned from the getHCPreSupportCode method.
    */
    static const std::string EquationSolvers;
};

#endif
)"
