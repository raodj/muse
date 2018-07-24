#ifndef SIMULATION_GENERATOR_CPP
#define SIMULATION_GENERATOR_CPP

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
#include "SimulationGenerator.h"
#include "AgentGenerator.h"
#include "MakefileGenerator.h"

std::string
SimulationGenerator::replaceAll(const EDL_AST& ast,
                                const std::string& srcTemplate) const {
    // Replace the simulation class name
    const std::string simName = ast.agentClassName + "Simulation";
    std::string source = replace(srcTemplate, "%CLASS_NAME%", simName);
    // Replace the agent class name
    source = replace(source, "%DISEASE_NAME%", ast.agentClassName);
    return source;
}

void
SimulationGenerator::generateEqnSolvers() {
    std::ofstream oclHdr("OclEqnSolvers.h"), oclSrc("OclEqnSolvers.c"),
        oclInc("OclEqnSolvers.c_inc");
    if (!oclHdr.good() || !oclSrc.good() || !oclInc.good()) {
        std::cerr << "Error opening one of the following files for writing: "
                  << "OclEqnSolvers.c, OclEqnSolvers.c_inc, or OclEqnSolvers.h"
                  << std::endl;
        return;
    }
    // Write fixed data to the 3 files
    oclHdr << OclEqnSolversHeader << std::endl;
    oclSrc << OclEqnSolversSource << std::endl;
    // Create an include file (C++11 raw string) used for generating
    // OpenCL source.
    oclInc << "R\"(" << OclEqnSolversSource << ")\"" << std::endl;
}

void
SimulationGenerator::generateSim(const EDL_AST& ast) {
    // Generate support files.
    generateEqnSolvers();
    // Generate the state and agent class
    AgentGenerator agentGen;
    agentGen.generateAgent(ast);
    // Generate the top-level simulation class.
    const std::string simName     = ast.agentClassName + "Simulation";
    const std::string headerName  = simName + ".h";
    const std::string sourceName  = simName + ".cpp";
    std::ofstream simHeader(headerName), simSrc(sourceName);
    if (!simHeader.good() || !simSrc.good()) {
        std::cerr << "Error opening files " << headerName << " or "
                  << sourceName << " for writing.\n";
        return;
    }
    // Generate the source file by substituting templates
    simHeader << replaceAll(ast, SimulationHeaderTemplate) << std::endl;
    simSrc    << replaceAll(ast, SimulationSourceTemplate) << std::endl;
    // Finally generate a makefile to build the generated source
    MakefileGenerator makeGen;
    makeGen.generateMakefile(ast);
}


// Include the agent's header template for use during code generation.
const std::string SimulationGenerator::SimulationHeaderTemplate =
#include "SimulationTemplate.h.inc"
;  // Trailing semicolon important!

// Include the agent's source template for use during code generation.
const std::string SimulationGenerator::SimulationSourceTemplate =
#include "SimulationTemplate.cpp.inc"
;  // Trailing semicolon important!

// Include the equation solvers's header source code for use during
// code generation.
const std::string SimulationGenerator::OclEqnSolversHeader =
#include "OclEqnSolvers.h.inc"
;  // Trailing semicolon important!

// Include the equation solvers's source code for use during code
// generation.
const std::string SimulationGenerator::OclEqnSolversSource =
#include "OclEqnSolvers.c.inc"
;  // Trailing semicolon important!

#endif
