#ifndef MAKEFILE_GENERATOR_CPP
#define MAKEFILE_GENERATOR_CPP

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
#include "MakefileGenerator.h"

void
MakefileGenerator::generateMakefile(const EDL_AST& ast) {
    const std::string stateName = ast.agentClassName + "State";
    const std::string fileName  = stateName + ".h";
    std::ofstream makefile("Makefile");
    if (!makefile.good()) {
        std::cerr << "Error opening Makefile for writing\n";
        return;
    }
    // Generate makefile with 1 substituted value.
    makefile << replace(MakefileTemplate, "%MODEL%", ast.agentClassName);
}

// Include the StateTemplate source code used when generating state
// for an epidemic.
const std::string MakefileGenerator::MakefileTemplate =
#include "MakefileTemplate.inc"
;  // Trailing semicolon important!

#endif
