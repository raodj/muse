#ifndef DEFAULT_SIMUALTION_H
#define DEFAULT_SIMUALTION_H

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include "Simulation.h"

BEGIN_NAMESPACE(muse);

/** The default implementation for Simulation.

    <p>This is the default, single-threaded implementation for a MUSE
    Simulation class.  Prior to Fall'17 was the only implementation
    available and was directly implemented in the base Simulation
    class.  As of Fall'17 the Simulation class is now deemed as an
    interface with different implementations for the actual
    Simulation.</p>

    \note Do not instantiate this class; instead use the
    Simulation::getSimulator() interface method to obtain a pointer to
    the relevant simulation object.

    <p>This class provides a relatively straightforward implementation
    for a single-threaded kernel -- i.e., one thread per MPI-process.
    The single thread essentially performs the following</p>
*/
class DefaultSimulation : public muse::Simulation {
    // Define friend so that initializeSimulation() static method can
    // create an instance of this class.
    friend class muse::Simulation;
protected:
    /** \brief Complete initialization of the Simulation

        Once the simulation instance is created, it must be fully
        initialized.  Notably, this includes initializing
        MPI. Afterwards, important instance variables such as the
        SimulatorID are set.

        \note This method may throw an exception if errors are
        encountered during initialization.
        
	\param argc[in,out] The number of command line arguments.
	This is the value passed-in to main() method.  This value is
	modifed if command-line arguments are consumed.
        
	\param argv[in,out] The actual command line arguments.  This
	list is modifed if command-line arguments are consumed by the
	kernel.

        \param initMPI[in] Flag to indicate if MPI needs to be
        reinitialized.  This flag is set to false if the simulation is
        simply being repeated and initialization of MPI is not
        necessary.
    */
    void initialize(int& argc, char* argv[], bool initMPI = true) override;
    
    /** \brief Finalize the Simulation

        This method will cause every Agent in the system to finalize,
        and will then delete the GVTManager and finalize the
        communicator.

        This method will also print statistics about the simulation.

        \param[in] stopMPI If this flag is true, then MPI is
        finalized.  Otherwise MPI is not finalized permitting another
        simulation run using current setup.
        
        \param[in] delCommMgr If this flag is true, then the
        communication manager is deleted.  Deletion of communication
        manager may be disabled by derived classes (if it is shared or
        needs to be used for other tasks).
    */
    void finalize(bool stopMPI = true, bool delCommMgr = true) override;

    /** \brief Convenience method to perform initialization/setup just
        before agents are initialized.

        This is a convenience method that derived classes can override
        to perform any additional initialization/setup operations
        prior to commencement of simulation.  This method calls the
        base class implementation to perform common setup.  It then
        finalizes the agent map by calling
        Communicator::registerAgents(allAgents)
    */
    virtual void preStartInit() override;
    
private:
    /** The only constructor for this class.

        The constructor merely initializes all the pointers and
        instance variables to their default initial value.
        
        \note The constructor is intentionally private to enfore singleton
        pattern.  Instead call, Simulation::getSimulation() method to
        obtain a valid instance of the simulation object being used to
        manage Agents (or Logical Processes).
    */
    DefaultSimulation();

    /** Unimplemented constructor to disable copying Simulation
        objects.

        This constructor is declared private but is intentionally
        undefined. So any attempt to copy an object of this class will
        result in compiler/linker errors.
    */
    DefaultSimulation(const Simulation&);

    /** Unimplemented assignment operator to disable copying
        Simulation objects.

        This constructor is declared private but is intentionally
        undefined. So any attempt to copy an object of this class will
        result in compiler/linker errors.
    */    
    DefaultSimulation& operator=(const Simulation&);

    /** The destructor.

     */
    virtual ~DefaultSimulation();    
};

END_NAMESPACE(muse);

#endif
