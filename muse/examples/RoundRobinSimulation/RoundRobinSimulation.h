#ifndef ROUND_ROBIN_SIMULATION_H
#define ROUND_ROBIN_SIMULATION_H

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

/** Top-level class to coordinate various simulation tasks.

    <p>This class coordinates the activities of the round robin
    simulation in which each agent forwards a token (single event) to
    its adjacent neighboring agent.  Agents are assumed to be
    organized in a logical circle.  Agent number 0 (zero) initiates
    the token/event that is circulated in a cycle.  A the end of each
    cycle, agent 0 prints a message indicating the token has completed
    another round.</p>
    
    This class represents the top-level class that coordinates the
    various activites involved in setting-up a round robin simulation,
    namely:

    <ol>

    <li>Parsing command-line arguments to override default
    settings.</li>

    <li>Creating the various agents inovlved in the simulation.</li>

    <li>Starting the simulation.</li>
    
    </ol>
*/
class RoundRobinSimulation {
public:
    /** Destructor.
        
        Currently the destructor for this class does not have much
        operation to perform and is merely peresent to adhere to
        conventions and serve as a place holder for future changes.
    */
    ~RoundRobinSimulation();
        
    /** The main interface method for this class that should be used
        to start and run the Simulation.  The command-line arguments
        to the program should be passed-in as the arguments to this
        method.  These values are typically the default values passed
        to a standard C/C++ main() function.
 
        \param[in] argc The number of command-line arguments.
 
        \param[in] argv The actual command-line arguments.
    */    
    static void run(int argc, char** argv);
    
protected:
    /** The default constructor.
        
        The default constructor is intentionally protected as this
        class is not meant to be directly instantiated.  Instead the
        RoundRobinSimulation::run() static method should be used to
        run the simulation.  The constructor initializes the
        simulation configuration variables to default initial values.
    */  
    RoundRobinSimulation();
    
    /** Helper method to process command-line arguments (if any).
 
        This method uses the ArgParser utility class to parse any
        command-line arguments supplied to the simulation and updates
        the configuration (several private instance variables)
        variables in this class.  The configuration variables are used
        by various method in the class to create various agents and
        initialize the simulation to match the supplied configuration.
 
        \param[in] argc The number of command-line arguments.
         
        \param[in] argv The actual command-line arguments.
    */
    void processArgs(int argc, char** argv);
    
    /** This method is used to create the agents in the simulation and
        register with the simulation kernel.  The number of agents
        created by this method is max_agents / max_nodes (because the
        agents are evenly distributed across the various compute nodes
        used for simulation).  The method also handles the edge case
        where the last compute node gets any additional agents (if
        max_agents is not evenly divisible by max_nodes).
        
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

private:
    /** The number of agents to be created in the simulation. This is
	the total number of agents in the simulation.  Each compute
	node used for simulation contains about max_agents / max_nodes
	number of agents.  This value can be modified using the \c
	--agents command-line argument.
    */
    int max_agents;

    /** The number of compute nodes being used for simulation. This
	parameter must match the value for the \c mpiexec \c -n
	command-line argument.  This value can be modified using the
	\c --nodes command-line argument.
    */    
    int max_nodes;

    /** The simulation-time when the simulation must logically
	terminate.

	Until this time is reached the agents will continue to
	circulate the token to adjacent agents in the simulation.
	This value can be modified using the \c --endTime command-line
	argument.
    */
    int end_time;

    /** Additional information to be displayed along with help
	message.

	This is a simple copyright type informational message that is
	displayed to the user when the \c --help command-line argument
	is used.
    */
    static const std::string HelpInfo;
};

#endif
