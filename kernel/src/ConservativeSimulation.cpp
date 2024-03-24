//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Jingbin Yu             yuj53@miamioh.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "ConservativeSimulation.h"
#include "ArgParser.h"
#include <iostream>

muse::ConservativeSimulation::ConservativeSimulation(): lookAhead(1) {

}

muse::ConservativeSimulation::~ConservativeSimulation() {

}

void muse::ConservativeSimulation::initialize(int& argc, char* argv[], bool initMPI) {
    // Parse arguments from command line
    parseCommandLineArgs(argc, argv);

    // Call base initialize
    Simulation::initialize(argc, argv, initMPI);
}

void muse::ConservativeSimulation::parseCommandLineArgs(int& argc, char* argv[]) {
    // First. parse base Simulation args and get rid of them
    muse::Simulation::parseCommandLineArgs(argc, argv);
    int cmdLookahead = 1;

    ArgParser::ArgRecord arg_list[] = {
        {"--lookahead", "The constant lookahead value",
            &cmdLookahead, ArgParser::INTEGER},
        {"", "", NULL, ArgParser::INVALID}
    };

    ArgParser argp(arg_list);
    argp.parseArguments(argc, argv, false);

    if (cmdLookahead <= 0) {
        cmdLookahead = 1;
        std::cerr << "look ahead value must be positive, using 1 instead" << std::endl;
    }

    lookAhead = cmdLookahead;

    DEBUG(std::cout << "Using conservative synchronization strategy, setting lookahead to " 
    << lookAhead << std::endl;);
}