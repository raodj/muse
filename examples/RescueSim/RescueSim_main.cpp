
/*
   Auto generated main class by muse code generator.
   This source does some of the setup for you.
   Check out musesimulation.org for the API to see Simulation class methods.
*/

#include "Simulation.h"
#include "DataTypes.h"
#include <cmath>
#include <cstdlib>
#include "arg_parser.h"
#include "PlayerDataTypes.h"
#include <vector>
#include <ostream>
#include "oSimStream.h"
#include "MTRandom.h"
using namespace muse;

/* The main
 */
int main(int argc, char** argv) {
    //first get simulation kernel instance to work with
    Simulation * kernel = Simulation::getSimulator();

    //now lets initialize the kernel
    kernel->initialize(argc,argv);


    //..........register agents and do stuff here..................
    std::cout << "Muse Linked and Compiled OK :-)" << std::endl;

    //we set the start and end time of the simulation here
    Time start=0, end=100;
    kernel->setStartTime(start);
    kernel->setStopTime(end);

    //we finally start the simulation here!!
    kernel->start();

    //now we finalize the kernel to make sure it cleans up.
    kernel->finalize();

    return (0);
}
