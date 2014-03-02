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

class RescueSim_main {
public:
   static void run(int &argc, char* argv[]);
   ~RescueSim_main();
protected:
   RescueSim_main();
   void processArgs(int &argc, char* argv[]);
   void createArea(int startNum, int endNum, int rank);
   void createVols(int startNum, int endNum, int rank);
   void createVics(int startNum, int endNum, int rank);
   void simulate();
   int max_nodes;
   int cols;
   int rows;
   int vols;
   int vics;
   int CCC_x;
   int CCC_y;
   int end_time;
   int vert_emph;
   int horz_emph;
};