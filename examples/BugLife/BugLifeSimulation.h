#ifndef BUG_LIFE_SIMULATION_H
#define BUG_LIFE_SIMULATION_H

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

#include "Simulation.h"
#include "BugDataTypes.h"

/** The top-level simulation class that coordinates various tasks.

    This class encapsulates the activites associated with the
    top-level of the BugLife simulation.  The class has been designed
    to run as a singleton class -- that is, the class cannot be
    instantiated.  Instead, the static BugLifeSimulation::run() method
    must be invoked to trigger various operations.  The methods in this
    class collectively perform the following tasks:

    - processArgs: This method is used to process any command-line
      arguments passed to the simulation.

    - createMap: This method is used to create coordinate mappings
      that map a given agent to a specific coordinate during
      simulation.

    - createSpaces: This method is used to create the space agents in
      the simulation.  The number of space agents is determined using
      rows * cols (command-line arguments to the simulation).

    - createBugs: This method is used to create the bug agents
      involved in the siulation.

    - simulate: This method is used to setup start and end times for
      simulation and run the actual simulation.
 */
class BugLifeSimulation {
public:
    /** The main interface method for this class that should be used
        to start and run the bug life simulation.  The command-line
        arguments to the program should be passed-in as the arguments
        to this method.  These values are typically the default values
        passed to a standard C/C++ main() function.

        \param[in] argc The number of command-line arguments.

        \param[in] argv The actual command-line arguments.
    */
    static void run(int &argc, char* argv[]);

    /** Destructor.

	Currently the destructor for this class does not have much
	operation to perform and is merely peresent to adhere to
	conventions and serve as a place holder for future changes.
     */
    ~BugLifeSimulation();
    
protected:
    /** The default constructor.

        The default constructor is intentionally protected as this
        class is not meant to be directly instantiated.  Instead the
        BugLifeSimulation::run() static method should be used to run
        the simulation.  The constructor initializes the simulation
        configuration variables to default initial values.
    */
    BugLifeSimulation();

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
    void processArgs(int& argc, char *argv[]);

    /** Convenience method to create a 2-D data structure to track
	location of various agents.

	This method is a refactored utility method to create and
	populate initial set of values in the coord_map instance
	variable.  The coord_map is a hash map to map location (x and
	y coordinate) to a given agent ID.

	\note This method uses the row and column instance variables
	for its operations.  Consequently, it should be called only
	after command-line arguments have already been processed.
    */
    void createMap();

    /** This method is used to create the space agents in the
	simulation and register with the simulation kernel.  The
	number of spaces agents created by this method is rows * cols
	/ max_nodes (because the space agents are evenly distributed
	across the various compute nodes used for simulation).  The
	method also handles the edge case where the last compute node
	gets any additional spaces (if the numbers are not evenly
	divisible).

	\note This method must be called only after the simulation
	kernel has already been initialized.
    */
    void createSpaces();

    /** This method is used to create the bug agents in the simulation
	and register with the simulation kernel.  The number of bug
	agents created by this method is bugs / max_nodes (because the
	bug agents are evenly distributed across the various compute
	nodes used for simulation).  The method also handles the edge
	case where the last compute node gets any additional bugs (if
	bugs is not evenly divisible by max_nodes).
	
	\note This method must be called only after the simulation
	kernel has already been initialized.
    */
    void createBugs();

    /**
       Convenience method to setup time duration for simulation and
       initiate the actual simulation.  This is a convenience method
       that is primarily used to streamline the top-level run() method
       in this class.
    */
    void simulate();
    
private:
    int cols;      
    int rows;
    int bugs;
    int max_nodes;
    int end_time;
    CoordAgentIDMap coord_map;
};

#endif
