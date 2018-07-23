#ifndef OCL_SYNTH_SIMULATION_H
#define OCL_SYNTH_SIMULATION_H

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

#include "Simulation.h"
#include <vector>

class SynthSimulation {
public:
    /** Destructor.
        
        Currently the destructor for this class does not have much
        operation to perform and is merely peresent to adhere to
        conventions and serve as a place holder for future changes.
    */
    ~SynthSimulation();
        
    /** The main interface method for this class that should be used
        to start and run the OCL Simulation.  The command-line
        arguments to the program should be passed-in as the arguments
        to this method.  These values are typically the default values
        passed to a standard C/C++ main() function.
 
        \param[in] argc The number of command-line arguments.
 
        \param[in] argv The actual command-line arguments.
    */    
    static void run(int argc, char** argv);

protected:
    /** The default constructor.
        
        The default constructor is intentionally protected as this
        class is not meant to be directly instantiated.  Instead the
        SynthSimulation::run() static method should be used to
        run the simulation.  The constructor initializes the
        simulation configuration variables to default initial values.
    */  
    SynthSimulation();

    /** Helper method to process command-line arguments (if any).
 
        This method uses the ArgParser utility class to parse any
        command-line arguments supplied to the simulation and updates
        the configuration (several private instance variables)
        variables in this class.  The configuration variables are used
        by various method in the class to create various agents and
        initialize the simulation to match the supplied configuration.
 
        \param[in] argc The number of command-line arguments.
         
        \param[in] argv The actual command-line arguments.

        \return This method returns true if the command-line arguments
        were successfully processed.
    */
    void processArgs(int& argc, char* argv[]);

    /** This method is used to create the agents in the simulation and
        register with the simulation kernel.  
        
        \note This method must be called only after the simulation
        kernel has already been initialized.
    */
    void createAgents();

    /**
       Convenience method to setup time duration for simulation and
       initiate the actual simulation.  This is a convenience method
       that is primarily used to streamline the top-level run() method
       in this class.
    */
    void simulate();

    /** Helper method to compute cumulative sum upto a given value.
        
        Convenience method to compute cumulative sum of series 1 + 2 +
        ... + val
        
        \param[in] val The value for which cumulative sum of series is
        to be returned.
        
        \param[in] scale The scale for multiplying the cumulative sum.
        
        \return This method returns scale * val * (val + 1) / 2.
    */
    int cumlSum(const int val, const double scale = 1) const {
        return (scale * val * (val - 1) / 2);
    }

private:
    /** Flag to indicate if ODE, SSA, SSA+Tau-Leap is to be run.  This
        value is set via the \c --solver command-line parameter.
    */
    int solver;

    /** The number of compartments to be used for operations */
    int compartments = 4;

    /** The number of parameters to be used for operations */
    int params = 4;

    /** The number of rows in the model to simulate */
    int rows = 3;

    /** The number of columns in the model to simulate */
    int cols = 3;

    /** The step size to eb used for solving ODE or SSA */
    double step = 0.01;

    /** The time when the simulation ends */
    int endTime = 100;

    /** Initial number of susceptible individuals in each agent */
    int susceptible = 1000;

    /** Initial number of exposed individuals in each agent */
    int exposed = 10;

    /** Flag to indicate if agents should print epidemic values at the
        end of simulation */
    bool showEpiVals = false;

    /** Number of events each agent should generate in each time step */
    int eventsPerLP;

    /** Granularity for each event event. Approximately 1 is equal to
        1 microsecond */
    int granularity;
};

#endif
