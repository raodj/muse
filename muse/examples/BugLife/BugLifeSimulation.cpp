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
#include <vector>
#include "SpaceState.h"
#include "BugState.h"
#include "Space.h"
#include "Bug.h"

using namespace muse;

// These must be static or global scope...
int cols;      
int rows;
int bugs;
int max_nodes;
int end_time;

//let make the arg_record
arg_parser::arg_record arg_list[] = {
    { "-cols","The Number of columns in the space.", &cols, arg_parser::INTEGER }, 
    { "-rows","The Number of rows in the space.", &rows, arg_parser::INTEGER },
    { "-bugs","The number of bugs you want in the simulation.", &bugs, arg_parser::INTEGER },
    { "-nodes","The max numbers of nodes used for this simulation.", &max_nodes, arg_parser::INTEGER },
    { "-end","The end time for the simulation.", &end_time, arg_parser::INTEGER },
    
    { NULL, NULL }
};


/* The main
 */
int main(int argc, char** argv) {
    
    //default values for parameters
    cols        = 10;   
    rows        = 10;   
    bugs        = 30;
    max_nodes   = 1;
    end_time    = 10;
    arg_parser ap( arg_list );
    ap.check_args( argc, argv ,true);

    
    //first get simulation kernel instance to work with
    Simulation * kernel = Simulation::getSimulator();
    
    //now lets initialize the kernel
    kernel->initialize(argc,argv);
    
    int max_space_agents       = cols*rows;
    int space_agents_per_node  = max_space_agents/max_nodes;
    int bug_agents_per_node    = bugs/max_nodes;
    int rank                   = 0;//kernel->getSimulatorID(); 

    ASSERT(max_space_agents >= bugs ); //make sure we have space for all the bugs

    //create the location to agent id mapping here
    vector<coord*> coords;
    CoordAgentIDMap coord_map;
    AgentID id = 0;
    for (int x=0;x < cols; x++ ){
        for (int y=0;y < rows; y++){
            coord location(x,y);
            coord_map[location] = id;
            //cout << "(" << location.first << "," << location.second << ") -> " << id;
            id++;
        }
        //cout <<endl;
    }

    if ( rank == (max_nodes-1) && (max_space_agents % max_nodes) > 0 ){
        //means we have to add the extra agents to the last kernel
        space_agents_per_node = (max_space_agents/max_nodes) + (max_space_agents % max_nodes);
        bug_agents_per_node   = (bugs/max_nodes) + (bugs % max_nodes);
    }

    //lets create space agents and register them to kernels
    AgentID space_id = -1u;
    for (AgentID i= 0;i <space_agents_per_node ; i++){
        SpaceState *ss = new SpaceState();
        space_id =  (max_space_agents/max_nodes)*rank + i;
        Space * space = new Space(space_id,ss);
        kernel->registerAgent(space);
    }//end for

    //now we need to create and register the bugs
    AgentID bug_id = -1u;
    for (AgentID i= max_space_agents;i <bug_agents_per_node ; i++){
        BugState *bs = new BugState();
        bug_id =  (bugs/max_nodes)*rank + i;
        Bug * bug = new Bug(bug_id,bs, &coord_map);
        kernel->registerAgent(bug);
    }//end for


    
    //we set the start and end time of the simulation here
    Time start=0, end=end_time;
    kernel->setStartTime(start);
    kernel->setStopTime(end);
    
    //we finally start the ping pong simulation here!!
    kernel->start();
    
    //now we finalize the kernel to make sure it cleans up.
    kernel->finalize();
    
    return (0);
}

