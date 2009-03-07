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

using namespace muse;

// These must be static or global scope...
bool arg1;      
bool arg2;
char *arg3;
//let make the arg_record
 arg_parser::arg_record arg_list[] = {
	{ "-arg1", &arg1, arg_parser::BOOLEAN }, 
	{ "-arg2", &arg2, arg_parser::BOOLEAN },
	{ "-arg3", &arg3, arg_parser::STRING },
	{ NULL, NULL }
 };

/*
 */
int main(int argc, char** argv) {
    
     arg1 = true;    // default initialization must occur before the
     arg2 = false;   // arg_parser is called!
     arg3 = NULL;

     arg_parser ap( arg_list );
     ap.check_args( argc, argv );

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

