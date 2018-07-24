#ifndef STATE_GENERATOR_CPP
#define STATE_GENERATOR_CPP

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
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include "StateGenerator.h"

std::string
StateGenerator::generateState(const EDL_AST& ast) {
    const std::string stateName = ast.agentClassName + "State";
    const std::string fileName  = stateName + ".h";
    std::ofstream stateFile(fileName);
    if (!stateFile.good()) {
        std::cerr << "Error opening state file " << fileName
                  << " for writing.\n";
    } else {
        // The only thing required to do for state is set the number
        // of compartments to be used.
        std::string stateSrc = replace(StateTemplate, "%CLASS_NAME%",
                                       stateName);
        stateSrc = replace(stateSrc, "%COMPS%",
                           std::to_string(ast.compartments.size()));
        stateFile << stateSrc << std::endl;
    }
    // Return the name of the state.
    return stateName;
}

// Include the StateTemplate source code used when generating state
// for an epidemic.
const std::string StateGenerator::StateTemplate =
#include "StateTemplate.h.inc"
;  // Trailing semicolon important!

#endif
