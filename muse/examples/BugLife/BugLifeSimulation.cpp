/* 
   This is a template to quickly get going with development
   The usuless task of setting up the bases classes is done for you.
   Just change names around and get to the good part.
*/



#include "Simulation.h"
#include "DataTypes.h"
#include <math.h>
#include <cstdlib>
#include "arg_parser.h"
#include "BugDataTypes.h"

using namespace muse;

// These must be static or global scope...
int x;      
int y;
int bugs;
int max_nodes;
int end_time;

//let make the arg_record
arg_parser::arg_record arg_list[] = {
    { "-x","The Number of columns in the space.", &x, arg_parser::INTEGER }, 
    { "-y","The Number of rows in the space.", &y, arg_parser::INTEGER },
    { "-bugs","The number of bugs you want in the simulation.", &bugs, arg_parser::INTEGER },
    { "-nodes","The max numbers of nodes used for this simulation.", &max_nodes, arg_parser::INTEGER },
    { "-end","The end time for the simulation.", &end_time, arg_parser::INTEGER },
    { NULL, NULL }
};


/* The main
 */
int main(int argc, char** argv) {
    
    //default values for parameters
    x        = 10;   
    y        = 10;   
    bugs     = 30;
    max_nodes= 1;
    end_time = 10;
    arg_parser ap( arg_list );
    ap.check_args( argc, argv ,true);
    
    //now its time to make the space agents and create the coord,id hashmap
    CoordAgentIDMap coord_map;
    

    //first get simulation kernel instance to work with
    //Simulation * kernel = Simulation::getSimulator();
    
    //now lets initialize the kernel
    //kernel->initialize(argc,argv);

    /** 
	.................code here.................
    */

    //kernel->registerAgent(agents[kernel->getSimulatorID()]);
    //we set the start and end time of the simulation here
    //Time start=0, end=0;
    //kernel->setStartTime(start);
    //kernel->setStopTime(end);
    
    //we finally start the ping pong simulation here!!
    //kernel->start();
    
    //now we finalize the kernel to make sure it cleans up.
    //kernel->finalize();
    
    return (0);
}

