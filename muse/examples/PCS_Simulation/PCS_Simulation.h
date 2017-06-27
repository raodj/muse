#ifndef PCS_SIMULATION_H
#define PCS_SIMULATION_H

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

/* 
 * File:   PCS_Simulation.h
 * Author: Julius Higiro
 *
 * Created on August 24, 2016, 12:29 PM
 */

#include "PCSAgent.h"
#include "PCS_State.h"

class PCS_Simulation {
public:
    /** Destructor.
        
        Currently the destructor for this class does not have much
        operation to perform and is merely present to adhere to
        conventions and serve as a place holder for future changes.
    */
    ~PCS_Simulation();
        
    /** The main interface method for this class that should be used
        to start and run the PCS Simulation.  The command-line
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
        PCS Simulation::run() static method should be used to run
        the simulation.  The constructor initializes the simulation
        configuration variables to default initial values.
    */  
    PCS_Simulation();
    
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
    bool processArgs(int argc, char** argv);
    
    /** This method is used to create the pcs agents in the
        simulation and register with the simulation kernel.  The
        number of pcs agents created by this method is max_agents /
        max_nodes (because the pcs agents are evenly distributed
        across the various compute nodes used for simulation).  The
        method also handles the edge case where the last compute node
        gets any additional agents (if max_agents is not evenly
        divisible by max_nodes).
        
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
    int cumlSum(const int val, const double scale = 1) const;
    
private:
    /** Percentage of imbalance required in initial partitioning.

        This value is a percentage in the range 0 to 0.99 (99%) that
        is specified as a command-line argument \c --imbalance.  This
        value is used to skew the even initial partitioning by the
        specified percentage. For example, given 1000 agents, 0.25
        imbalance, on 4 processes, the partition of agents will be
        {188, 229, 271, 312} (i.e., 250 agents were distributed to the
        4 processes weighted by the rank of the processes).
    */
    double imbalance;

    /** Fraction of events that agents should schedule to themselves.

        This value indicates the probability that agents should
        schedule events to themselves.  This value is in the range 0.0
        to 0.99 and is used with the boolean expression ((rand() %
        1000) / 1000.0) < selfEvents.  If the boolean expression
        returns true, then agents schedule events to themselves.  Note
        that at a given virtual time, the maximum number of
        self-events is limited by events * selfEvents.
    */
    double selfEvents;

    /** A user-specified granularity -- that is, the number of dummy
        loops to run by each agent for each event to add some
        CPU-load/granularity for each event.

        This instance variable is set to indicate the number of loops
        or iterations that must be executed by the agent to simulate
        some granularity, i.e., CPU used per event.  This value is set
        to zero by default and can be modified via the command-line
        argument \c --granularity.
    */
    size_t granularity;
    
    /** The number of columns in the grid of agents in PCS. This
        value is specified as a command-line argument \c --cols.
    */
    int cols;

    /** The number of rows in the grid of agents in PHold. This
        value is specified as a command-line argument \c --rows.
    */    
    int rows;

    /** The initial number of events generated by each agent in
        PCS. This value is specified as a command-line argument \c
        --eventsPerAgent.
    */     
    int events;

    /** Fixed lookahead virtual time delay for generating events.

        The virtual time events for each agent are generated which
        this fixed look ahead value.
    */
    int lookAhead;
    
    /** The maximum uniform-random delay to be added to the lookahead
        when scheduling events.  This value is not zero, then it used
        to compute a random delay factor as: lookAhead + (rand() %
        delay).
    */
    int delay;
    
    /** The virtual time when the simulation is deemed to be complete.
        This value is set via the command-line argument --endTime
    */
    int end_time;

    /** The MPI rank of this simulation process.  */
    int rank;
    
    /** The maximum number of channels that will assigned to each agent.
        This value is set via the command-line argument --maxChannels
    */
    int channels;
    
    /** The average call duration time.
        This value is set via the command-line argument --callIntervalMean
    */
    int callIntervalMean;
    
    /** The average call completion time. 
        This value is set via the command-line argument --callDurationMean
    */
    int callDurationMean;
    
    /** The average move time.
        This value is set via the command-line argument --moveIntervalMean
    */
    int moveIntervalMean;
};

#endif /* PCS_SIMULATION_H */
