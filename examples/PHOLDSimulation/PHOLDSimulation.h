#ifndef PHOLD_SIMULATION_H
#define PHOLD_SIMULATION_H

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
   Author: Meseret Gebre
   
   This is a Synthetic Simulation done for benchmarking. P-HOLD simulation is 
   meant to mimic a typical load for a given simulation and can be scaled.
   
   rows = Number of agents per row
   cols = Number of agents per column
   eventsPerAgent = Number of initial events each agent starts with
   delay = The Delay time for the receive time, this will be max range for a random from [0,1]
   computeNodes = The max number of nodes to run P-HOLD.
   simEndTime = The end time for the simulation.
   
   @note Please see PHOLDAgent.cpp for more detail :-)
   
   Object Orientation done by Jake Gregg
*/
class PHOLDSimulation {
public:
    /** Destructor.
        
        Currently the destructor for this class does not have much
        operation to perform and is merely peresent to adhere to
        conventions and serve as a place holder for future changes.
    */
    ~PHOLDSimulation();
        
    /** The main interface method for this class that should be used
        to start and run the PHOLD Simulation.  The command-line
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
        PHOLDSimulation::run() static method should be used to run
        the simulation.  The constructor initializes the simulation
        configuration variables to default initial values.
    */  
    PHOLDSimulation();
    
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
    
    /** This method is used to create the phold agents in the
        simulation and register with the simulation kernel.  The
        number of phold agents created by this method is max_agents /
        max_nodes (because the phold agents are evenly distributed
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
        is specifed as a command-line argument \c --imbalance.  This
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
    
    /** The number of columns in the grid of agents in PHold. This
        value is specified as a command-line argument \c --cols.
    */
    int cols;

    /** The number of rows in the grid of agents in PHold. This
        value is specified as a command-line argument \c --rows.
    */    
    int rows;

    /** The initial number of events generated by each agent in
        PHold. This value is specified as a command-line argument \c
        --eventsPerAgent.
    */     
    int events;

    /** Fixed lookahead virtual time delay for generating events.

        The virtual time events for each agent are generated which
        this fixed look ahead value.
    */
    int lookAhead;
    
    /** The maximum uniform-random/mean delay to be added to the
        lookahead when scheduling events.  This value is not zero,
        then it used to compute a random delay factor as: lookAhead +
        (rand() % delay).
    */
    int delay;

    /** The type of delay distribution to be for generating delays.
        Valid delay distributions are: uniform, poisson, exponential,
        reverse_poisson, reverse_exponential.  The reverse forms of
        the distribution are mirror images of the regular
        distributions causing a heavy tailed distribution.
    */
    std::string delayDistrib;
    
    /** The virtual time when the simulation is deemed to be complete.
        This value is set via the command-line argument --endTime
    */
    int end_time;

    /** The MPI rank of this simulation process.  */
    int rank;

    /** Flag to indicate if the delay histogram should be printed. */
    bool delayHist;

    /** Command line argument associated with the range to be used to
        determine the receiver agent ID.  The default value is
        zero. This value is essentially passed onto the agent objects
        created.
    */
    int receiverRange;

    /** The type of delay distribution to be for generating receiver
        agent IDs using receiverRange.  Valid delay distributions are:
        uniform, poisson, exponential, reverse_poisson,
        reverse_exponential, local_remote.  The reverse forms of the
        distribution are mirror images of the regular distributions
        causing a heavy tailed distribution.
    */
    std::string recvrDistrib;

    /** Additional bytes to be added to an event to facilitate
        characterization of models with different event sizes.  Note
        that this value refers to extra bytes (on top of the basic
        muse::Event size) to be added to each event.  Currently, the
        extra bytes added to events cannot be varied.  The increased
        event size several impacts:

        <ul>

        <li>The larger events introduce some additional event
        processing overheads associated simply scanning the
        bytes to simulate some access.</li>
        
        <li>It takes longer to exchange larger events over network or
        shared-memory</li>

        <li>It takes more memory to store larger events in memory.</li>
        
        </ul>
    */
    int extraEventSize;

    /** Fraction of remote events to be sent when receiver
        distribution is local_remote

        This command-line argument (\c --remote-events) determines the
        fraction of remote events generated by each agent.  This value
        is used by the agents only when the recvrDistrib is
        local_remote.  The default value is 0.
    */
    double remoteEvents;
};

#endif
