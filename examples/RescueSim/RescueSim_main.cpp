#include "Simulation.h"
#include "DataTypes.h"
#include <cmath>
#include <cstdlib>
#include "ArgParser.h"
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
#include <mpi.h>
using namespace muse;

int cols;
int rows;
int vols;
int vics;
int CCC_x;
int CCC_y;
int max_nodes;
int end_time;

ArgParser::ArgRecord arg_list[] = {
   { "-cols","The Number of columns in the space.", &cols, ArgParser::INTEGER }, 
   { "-rows","The Number of rows in the space.", &rows, ArgParser::INTEGER },
   { "-vols","The number of volunteers you want in the simulation.", &vols, ArgParser::INTEGER },
   { "-vics","The number of victims you want in the simulation.", &vics, ArgParser::INTEGER },
   { "-CCCx","The x-coordinate of the CCC.", &CCC_x, ArgParser::INTEGER },
   { "-CCCy","The y-coordinate of the CCC.", &CCC_y, ArgParser::INTEGER },
   { "-end","The end time for the simulation.", &end_time, ArgParser::INTEGER },
   { "", "", NULL, ArgParser::INVALID }
};

int main(int argc, char** argv) {
   srand(time(0));
   cols = 100;
   rows = 100;
   vols = 1;
   vics = 2;
   CCC_x = 0;
   CCC_y = 0;
   max_nodes = 1;
   end_time = 100;
   ArgParser ap(arg_list);
   ap.parseArguments(argc, argv, true);

   //first get simulation kernel instance to work with
   Simulation * kernel = Simulation::getSimulator();

   //now lets initialize the kernel
   kernel->initialize(argc,argv);
    
   int max_area_agents = cols/AREA_COL_WIDTH;
   int rank            = kernel->getSimulatorID(); 
   max_nodes           = kernel->getNumberOfProcesses();

   AgentID area_id = -1u;
   for (AgentID i = 0; i < max_area_agents; i++){
      if(i % max_nodes == rank) {
         RescueAreaState *ss = new RescueAreaState(i);
         area_id = i;
         RescueArea * space = new RescueArea(area_id, ss);
         std::cout << "Area agent " << i << " registered by node " << rank << std::endl;
         kernel->registerAgent(space);
      }
   }

   AgentID vol_id = -1u;
   for(AgentID i = max_area_agents; i < max_area_agents + vols; i++){
      if(i % max_nodes == rank) {
         VolunteerState *vs = new VolunteerState();
         vol_id = i;
         Volunteer * vol = new Volunteer(vol_id, vs, cols, rows, CCC_x, CCC_y);
         std::cout << "Vol agent " << i << " registered by node " << rank << std::endl;
         kernel->registerAgent(vol);
      }
   }

   AgentID vic_id = -1u;
   for(AgentID i = max_area_agents + vols; i < max_area_agents + vols + vics; i++){
      if(i % max_nodes == rank) {
         VictimState *vs = new VictimState();
         vic_id = i;
         Victim * vic = new Victim(vic_id, vs, cols, rows);
         std::cout << "Vic agent " << i << " registered by node " << rank << std::endl;
         kernel->registerAgent(vic);
      }
   }

   //we set the start and end time of the simulation here
   Time start=0, end=end_time;
   kernel->setStartTime(start);
   kernel->setStopTime(end);

   MPI_Barrier(MPI_COMM_WORLD);
   //we finally start the simulation here!!
   kernel->start();

   //now we finalize the kernel to make sure it cleans up.
   kernel->finalize();

   return (0);
}
