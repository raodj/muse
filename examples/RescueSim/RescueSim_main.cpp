#include "RescueSim_main.h"
using namespace muse;

RescueSim_main::RescueSim_main() {
   cols      = 100;
   rows      = 100;
   vols      = 1;
   vics      = 2;
   CCC_x     = 0;
   CCC_y     = 0;
   max_nodes = 1;
   end_time  = 100;
}

RescueSim_main::~RescueSim_main() { }

void
RescueSim_main::processArgs(int& argc, char* argv[]) {
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
   ArgParser ap(arg_list);
   ap.parseArguments(argc, argv, true);

   //first get simulation kernel instance to work with
   Simulation * const kernel = Simulation::getSimulator();
   //now lets initialize the kernel
   kernel->initialize(argc,argv);
}

void
RescueSim_main::createArea() {
   AgentID area_id           = -1u;
   int max_area_agents       = cols/AREA_COL_WIDTH;
   Simulation * const kernel = Simulation::getSimulator();
   int rank                  = kernel->getSimulatorID();
   for (AgentID i = 0; i < max_area_agents; i++){
      if(i % max_nodes == rank) {
         RescueAreaState *ss = new RescueAreaState(i);
         area_id             = i;
         RescueArea * space  = new RescueArea(area_id, ss);
         std::cout << "Area agent " << i << " registered by node " << rank << std::endl;
         kernel->registerAgent(space);
      }
   }
}

void
RescueSim_main::createVols() {
   AgentID vol_id            = -1u;
   int max_area_agents       = cols/AREA_COL_WIDTH;
   Simulation * const kernel = Simulation::getSimulator();
   int rank                  = kernel->getSimulatorID();
   for(AgentID i = max_area_agents; i < max_area_agents + vols; i++){
      if(i % max_nodes == rank) {
         VolunteerState *vs = new VolunteerState();
         vol_id             = i;
         Volunteer * vol    = new Volunteer(vol_id, vs, cols, rows, CCC_x, CCC_y);
         std::cout << "Vol agent " << i << " registered by node " << rank << std::endl;
         kernel->registerAgent(vol);
      }
   }
}

void
RescueSim_main::createVics() {
   AgentID vic_id            = -1u;
   int max_area_agents       = cols/AREA_COL_WIDTH;
   Simulation * const kernel = Simulation::getSimulator();
   int rank                  = kernel->getSimulatorID();
   for(AgentID i = max_area_agents + vols; i < max_area_agents + vols + vics; i++){
      if(i % max_nodes == rank) {
         VictimState *vs = new VictimState();
         vic_id          = i;
         Victim * vic    = new Victim(vic_id, vs, cols, rows);
         std::cout << "Vic agent " << i << " registered by node " << rank << std::endl;
         kernel->registerAgent(vic);
      }
   }
}

void
RescueSim_main::simulate() {
   //we set the start and end time of the simulation here
   Time start = 0, end = end_time;
   Simulation * const kernel = Simulation::getSimulator();
   kernel->setStartTime(start);
   kernel->setStopTime(end);

   MPI_Barrier(MPI_COMM_WORLD);
   //we finally start the simulation here!!
   kernel->start();

   //now we finalize the kernel to make sure it cleans up.
   kernel->finalize();
}

void
RescueSim_main::run(int& argc, char* argv[]) {
   srand(time(0));
   Simulation * const kernel = Simulation::getSimulator();
   RescueSim_main Rmain;
   Rmain.max_nodes = kernel->getNumberOfProcesses();
   Rmain.processArgs(argc, argv);
   Rmain.createArea();
   Rmain.createVols();
   Rmain.createVics();
   Rmain.simulate();
}

int
main(int argc, char** argv) {
   RescueSim_main::run(argc, argv);
   return 0;
}

