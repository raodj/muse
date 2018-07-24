#ifndef AGENT_GENERATOR_CPP
#define AGENT_GENERATOR_CPP

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
#include <sstream>
#include <fstream>
#include "AgentGenerator.h"
#include "StateGenerator.h"
#include "SimulationGenerator.h"

void
AgentGenerator::generateODEs(const EDL_AST& ast, const std::string& comp,
                            std::ostream& os) const {
    // Find all transitions for this 1 compartment and generate 1 ODE
    // for its state transitions.
    std::ostringstream ode;
    // Iterate over the transitions and use the ones associated with
    // this compartment.
    bool isFirstTerm = true;  // Flag to add a " + " to equation
    for (const Transition& tr : ast.transitionList) {
        Transition::const_iterator entry = tr.find(comp);
        if (entry != tr.end()) {
            // Generate the term in the ODE
            ode << (!isFirstTerm ? " + " : "") << "(" << entry->second
                << " * " << tr.rateExpr << ")";
            // Rest of the terms require a " + " to make it an equation
            isFirstTerm = false;
        }
    }
    // Generate final equation only if some transition as found
    if (!isFirstTerm) {
        // At least one entry was found.
        os << "    deltas[" << comp << "] = " << ode.str() << ";\n";
    }
}

void
AgentGenerator::generateTotalPop(const EDL_AST& ast, const std::string& var,
                                std::ostream& os,
                                const std::string indent) const {
    os << indent << "// " <<  var << " is automatically generated\n";
    // Generate the automagic constant 'N' as convenience.
    os << indent << "const real " << var << " = (";
    std::string plus = "";  // First term does not have " + " prefix
    for (const std::string& comp : ast.compartments) {
        os << plus << "comp[" << comp << "]";
        plus = " + ";
    }
    os << ");\n";
}

// Helper method to convert transitions into a system of Ordinary
// Differential Equations (ODEs)
void
AgentGenerator::generateODEs(const EDL_AST& ast, std::ostream& os) const {
    // Iterate over each compartment and generate ODE's for that
    // compartment.
    os << "void ode(const int numVars, const real params[],\n"
       << "         const real comp[], real deltas[]) {\n"
       << "    UNUSED_PARAM(numVars);\n"
       << "    UNUSED_PARAM(params);\n\n";

    os << "    // The set of ODEs (usually in agent's .cpp file)\n";
    // Generate the automagic constant 'N' as convenience.
    generateTotalPop(ast, "N", os);
    // Generate ODEs for each compartment
    for (const std::string& comp : ast.compartments) {
        generateODEs(ast, comp, os);
    }
    // A blank line at the end to separate code fragments
    os << "}\n\n";
}

// Convenience method to generate a list of compartments in slightly
// different formats
std::string
AgentGenerator::getCompartmentList(const EDL_AST& ast, const std::string delim,
                                  const std::string indent) const {
    std::ostringstream os;   // temporary storage
    // Generate list of compartments
    for (const std::string& comp : ast.compartments) {
        os << indent << comp << delim;
    }
    // Return the compartments as a string.
    return os.str();
}

// Generate enumeration of compartment values.  Typically this
// enumeration will be within the scope of the Agent class for the
// epidemic.
void
AgentGenerator::generateCompartments(const EDL_AST& ast,
                                    std::ostream& os) const {
    const std::string indent(8, ' ');
    os << "    // Compartments in model (usually in header file of agent)\n";
    os << "    enum CompartmentNames {\n";
    os << getCompartmentList(ast, ",\n", indent);
    os << indent << "INVALID\n"
       << "    };\n\n";
}

// Convenience method to generate a row in the transition matrix in
// the form: "{+1, 0, 0, 0}"
std::string
AgentGenerator::getCompTransitions(const EDL_AST& edl,
                                  const Transition& tr) const {
    std::ostringstream matrixRow;  // Temporary output location
    bool needComma = false;
    // Generate the row of matrix
    // matrixRow << "{";
    for (const std::string comp : edl.compartments) {
        Transition::const_iterator entry = tr.find(comp);
        const double change = (entry != tr.end() ? entry->second : 0.0);
        matrixRow << (needComma ? ", " : "") << change;
        // Subsequent entries need a comma!
        needComma = true;
    }
    // matrixRow << "}";
    return matrixRow.str();
}
    
// Helper method to generate the rate matrix to introduce changes in
// various compartments.
void
AgentGenerator::generateSSAMatrix(const EDL_AST& ast, std::ostream& os,
                                 const std::string& indent) const {
    os << indent
       << "// This matrix should be a static constant in the header\n";
    // Generate the rate matrix to be part of the agent class.
    os << indent << "constant real " << ast.agentClassName
       << "_EventChange[] = {\n";
    // Helpful list of compartments for verification by modeler
    // Generate transitions for each rate equation in the EDL
    const std::string subIndent = indent + "    ";    
    os << subIndent << "// " << getCompartmentList(ast, ", ") << std::endl;
    // In this code, the logic for generating a comma works better
    // with an index-based iteration rather than a range-based-for
    for (size_t tr = 0; (tr < ast.transitionList.size()); tr++) {
        // User helper method to generate each row of the matrix
        os << subIndent << getCompTransitions(ast, ast.transitionList[tr]);
        // Generate comma only if more transition are left.
        if (tr < ast.transitionList.size() - 1) {
            os << ",";
        }
        // Generate the rate associated with this entry for
        // verification by the modeler.
        os << "  // " << ast.transitionList[tr].rateExpr << std::endl;
    }
    os << indent << "};\n\n";
}

void
AgentGenerator::generateSSARates(const EDL_AST& ast, std::ostream& os,
                                const std::string& indent) const {
    os << indent << "// Rates for transitions in the rate matrix.\n"
       << indent << "// This method is used to determine rates\n"
       << indent << "// each time the SSA is to be run.\n";
    os << indent << "void getSSARates(const int numRates, "
       << "const real params[],\n"
       << indent << "          const real comp[], real rates[]) {\n"
       << indent << "    UNUSED_PARAM(numRates);\n"
       << indent << "    UNUSED_PARAM(params);\n\n";
    // Generate the rates. But first generate the automatic 'N'
    generateTotalPop(ast, "N", os, indent);
    const std::string subIndent = indent + "    ";
    int i = 0;
    for (const Transition& tr : ast.transitionList) {
        os << subIndent << "rates[" << i << "] = "
           << tr.rateExpr << ";\n";
        i++;
    }
    os << std::endl << indent << "};\n\n";
}

// Convenience method to generate fixed constants setup by the user.
void
AgentGenerator::generateConstants(const EDL_AST& ast, std::ostream& os,
                                 const std::string& indent) const {
    os << indent << "// Constants used in both ODE & SSA\n";
    // Generate code for each constants in the HashMap in AST
    for (auto& entry : ast.constants) {
        os << indent << "constant real " << entry.first << " = "
           << entry.second << ";\n";
    }
    // A blank line at the end to separate code fragments
    os << std::endl;
}

void
AgentGenerator::generateAgentHeader(const EDL_AST& ast) const {
    const std::string agentName  = ast.agentClassName;
    const std::string headerName = agentName + "Agent.h";
    // Open the header file for writing.
    std::ofstream headerFile(headerName);    
    if (!headerFile.good()) {
        std::cerr << "Error opening file " << headerName
                  << " for writing.\n";
        return;
    }
    // Generate the header by substituting variables
    std::string src = replace(AgentHeaderTemplate, "%CLASS_NAME%", agentName);
    src = replace(src, "%PARAMS%", std::to_string(ast.parameters.size()));
    headerFile << src << std::endl;
}

void
AgentGenerator::generateAgentSource(const EDL_AST& ast) const {
    // Now generate the code to be plased in the agent class.
    std::stringstream os;
    generateConstants(ast, os);
    generateCompartments(ast, os);
    generateODEs(ast, os);
    generateSSAMatrix(ast, os);
    generateSSARates(ast, os);
    // Create the agent name
    const std::string agentName  = ast.agentClassName;
    const std::string sourceName = agentName + "Agent.cpp";
    // Generate the source file by substituting variables
    std::string src = replace(AgentSourceTemplate, "%CLASS_NAME%", agentName);
    src = replace(src, "%AGENT_HC_SOURCE%", os.str());
    src = replace(src, "%PARAMS%", std::to_string(ast.parameters.size()));
    // Write the source to file.
    std::ofstream sourceFile(sourceName);
    if (!sourceFile.good()) {
        std::cerr << "Error opening file " << sourceName
                  << " for writing.\n";
        return;
    }
    sourceFile << src << std::endl;
}

void
AgentGenerator::generateAgent(const EDL_AST& ast) {
    // Generate the state definition for the EDL epidemic
    StateGenerator sg;
    sg.generateState(ast);
    // Next, produce the header and C++ source files for the
    // generator.
    generateAgentHeader(ast);
    generateAgentSource(ast);
}

// Include the agent's header template for use during code generation.
const std::string AgentGenerator::AgentHeaderTemplate =
#include "AgentTemplate.h.inc"
;  // Trailing semicolon important!

// Include the agent's source template for use during code generation.
const std::string AgentGenerator::AgentSourceTemplate =
#include "AgentTemplate.cpp.inc"
;  // Trailing semicolon important!

#endif
