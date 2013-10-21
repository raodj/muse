#include "Simulation.h"
#include "DataTypes.h"
#include <cmath>
#include <cstdlib>
#include "arg_parser.h"
#include "VolunteerDataTypes.h"
#include <vector>
#include <ostream>
#include "oSimStream.h"
#include "MTRandom.h"
#include "RescueArea.h"
#include "RescueAreaState.h"
#include "Volunteer.h"
#include "VolunteerState.h"
#include "Victim.h"
#include "VictimState.h"
using namespace muse;

int cols;
int rows;
int vols;
int vics;
int CCC_x;
int CCC_y;
int max_nodes;
int end_time;

arg_parser::arg_record arg_list[] = {
   { "-cols","The Number of columns in the space.", &cols, arg_parser::INTEGER }, 
   { "-rows","The Number of rows in the space.", &rows, arg_parser::INTEGER },
   { "-vols","The number of volunteers you want in the simulation.", &vols, arg_parser::INTEGER },
   { "-vics","The number of victims you want in the simulation.", &vics, arg_parser::INTEGER },
   { "-CCCx","The x-coordinate of the CCC.", &CCC_x, arg_parser::INTEGER },
   { "-CCCy","The y-coordinate of the CCC.", &CCC_y, arg_parser::INTEGER },
   { "-nodes","The max numbers of nodes used for this simulation.", &max_nodes, arg_parser::INTEGER },
   { "-end","The end time for the simulation.", &end_time, arg_parser::INTEGER },
   { NULL, NULL }
};

int main(int argc, char** argv) {
   cols = 100;
   rows = 100;
   vols = 1;
   vics = 2;
   CCC_x = 0;
   CCC_y = 0;
   max_nodes = 1;
   end_time = 100;
   arg_parser ap(arg_list);
   ap.check_args(argc, argv, true);

   //first get simulation kernel instance to work with
   Simulation * kernel = Simulation::getSimulator();

   //now lets initialize the kernel
   kernel->initialize(argc,argv);
    
   int max_area_agents      = cols/AREA_COL_WIDTH;
   int area_agents_per_node = max_area_agents/max_nodes;
   int vol_agents_per_node  = vols/max_nodes;
   int vic_agents_per_node  = vics/max_nodes;
   int rank                 = kernel->getSimulatorID(); 

   if ( rank == (max_nodes-1) && (max_area_agents % max_nodes) > 0 ){
       area_agents_per_node = (max_area_agents/max_nodes) + (max_area_agents % max_nodes);
       vol_agents_per_node   = (vols/max_nodes) + (vols % max_nodes);
       vic_agents_per_node   = (vics/max_nodes) + (vics % max_nodes);
   }

   CoordAgentIDMap coord_map;
   AgentID id = 0;
   for(int y = 0; y < cols; y++) {
      coord_map[y] = id;
      if((y+1) % AREA_COL_WIDTH == 0) id++;
   }

   AgentID area_id = -1u;
   for (AgentID i = 0; i < area_agents_per_node; i++){
      RescueAreaState *ss = new RescueAreaState(i);
      area_id =  (max_area_agents/max_nodes)*rank + i;
      RescueArea * space = new RescueArea(area_id, ss);
      kernel->registerAgent(space);
   }

   AgentID vol_id = -1u;
   for(AgentID i = max_area_agents; i < max_area_agents + vol_agents_per_node; i++){
      VolunteerState *vs = new VolunteerState();
      vol_id =  (vols/max_nodes)*rank + i;
      Volunteer * vol = new Volunteer(vol_id, vs, &coord_map, cols, rows, CCC_x, CCC_y);
      kernel->registerAgent(vol);
   }

   AgentID vic_id = -1u;
   for(AgentID i = max_area_agents + vol_agents_per_node; i < max_area_agents + vol_agents_per_node + vic_agents_per_node; i++){
      VictimState *vs = new VictimState();
      vic_id =  (vics/max_nodes)*rank + i;
      Victim * vic = new Victim(vic_id, vs, &coord_map, cols, rows);
      kernel->registerAgent(vic);
   }

   //we set the start and end time of the simulation here
   Time start=0, end=end_time;
   kernel->setStartTime(start);
   kernel->setStopTime(end);

   //we finally start the simulation here!!
   kernel->start();

   //now we finalize the kernel to make sure it cleans up.
   kernel->finalize();

   return (0);
}
