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

#include <iostream>
#include "PHOLDAgent.h"
#include "Simulation.h"
#include "PholdState.h"
#include "DataTypes.h"
#include <math.h>
#include <cstdlib>
#include "ArgParser.h"

using namespace muse;
using namespace std;

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
        */
        void processArgs(int argc, char** argv);
        /** This method is used to create the phold agents in the simulation
            and register with the simulation kernel.  The number of phold
            agents created by this method is max_agents / max_nodes (because the
            phold agents are evenly distributed across the various compute
            nodes used for simulation).  The method also handles the edge
            case where the last compute node gets any additional agents (if
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
        int cols, rows, events, delay, max_nodes, end_time, max_agents, agentsPerNode, rank;

};

