#include "EdlAst.h"
#include <iostream>
#include <sstream>

void
generateODEs(const EDL_AST& ast, const std::string& comp,
             std::ostream& os) {
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
        os << "    comp[" << comp << "] = " << ode.str() << ";\n";
    }
}

void
generateTotalPop(const EDL_AST& ast, const std::string& var,
                 std::ostream& os, const std::string indent = "    ") {
    os << indent << "// " <<  var << " is automatically generated\n";
    // Generate the automagic constant 'N' as convenience.
    os << indent << "const " << var << " = (";
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
generateODEs(const EDL_AST& ast, std::ostream& os) {
    // Iterate over each compartment and generate ODE's for that
    // compartment.
    os << "    // The set of ODEs (usually in agent's .cpp file)\n";
    // Generate the automagic constant 'N' as convenience.
    generateTotalPop(ast, "N", os);
    // Generate ODEs for each compartment
    for (const std::string& comp : ast.compartments) {
        generateODEs(ast, comp, os);
    }
    // A blank line at the end to separate code fragments
    os << std::endl;    
}

// Convenience method to generate a list of compartments in slightly
// different formats
std::string
getCompartmentList(const EDL_AST& ast,
                   const std::string delim = ",\n",
                   const std::string indent = "") {
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
generateCompartments(const EDL_AST& ast, std::ostream& os) {
    const std::string indent(8, ' ');
    os << "    // Compartments in model (usually in header file of agent)\n";
    os << "    enum CompartmentNames = {\n";
    os << getCompartmentList(ast, ",\n", indent);
    os << indent << "INVALID\n"
       << "    };\n\n";
    os << "    // The vector that contains current compartment values\n"
       << "    // This vector should be in the agent's state\n"
       << "    std::vector<real> comp;\n\n";
}

// Convenience method to generate a row in the transition matrix in
// the form: "{+1, 0, 0, 0}"
std::string
getCompTransitions(const EDL_AST& edl, const Transition& tr) {
    std::ostringstream matrixRow;  // Temporary output location
    bool needComma = false;
    // Generate the row of matrix
    matrixRow << "{";
    for (const std::string comp : edl.compartments) {
        Transition::const_iterator entry = tr.find(comp);
        const double change = (entry != tr.end() ? entry->second : 0.0);
        matrixRow << (needComma ? ", " : "") << change;
        // Subsequent entries need a comma!
        needComma = true;
    }
    matrixRow << "}";
    return matrixRow.str();
}
    
// Helper method to generate the rate matrix to introduce changes in
// various compartments.
void
generateSSAMatrix(const EDL_AST& ast, std::ostream& os,
                  const std::string& indent = "    ") {
    os << indent
       << "// This matrix should be a static constant in the header\n";
    os << indent << "using RateMatrix = std::vector<std::vector<real>>;\n";
    // Generate the rate matrix to be part of the agent class.
    os << indent << "const RateMatrix " << ast.agentClassName
       << "::EventChange = {\n";
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
generateSSARates(const EDL_AST& ast, std::ostream& os,
                 const std::string& indent = "    ") {
    os << indent << "// Rates for transitions in the rate matrix.\n"
       << indent << "// This should be in nextSSA method to determine rates\n"
       << indent << "// each time the SSA is to be run.\n";
    // Generate the rates. But first generate the automatic 'N'
    generateTotalPop(ast, "N", os, indent);
    os << indent << "const std::vector<real> rates = {\n";
    const std::string subIndent = indent + "    ";
    bool needComma = false;
    for (const Transition& tr : ast.transitionList) {
        os << (needComma ? ",\n" : "")    // Finish previous line
           << subIndent << tr.rateExpr;
        needComma = true;
    }
    os << std::endl << indent << "}\n\n";
}

// Convenience method to generate fixed constants setup by the user.
void
generateConstants(const EDL_AST& ast, std::ostream& os,
                  const std::string& indent = "    ") {
    os << indent << "// Constants used in both ODE & SSA\n";
    // Generate code for each constants in the HashMap in AST
    for (auto& entry : ast.constants) {
        os << indent << "const real " << entry.first << " = "
           << entry.second << ";\n";
    }
    // A blank line at the end to separate code fragments
    os << std::endl;
}

void
generateCode(EDL_AST& ast, std::ostream& os) {
    generateConstants(ast, os);
    generateCompartments(ast, os);
    generateODEs(ast, os);
    generateSSAMatrix(ast, os);
    generateSSARates(ast, os);
    os << "// Code generation is not yet fully implemented.\n";
    os << "// Please copy-paste code appropriately.\n";
}

// Top-level method that is called at the end of successful parsing to
// generate code using information in the Abstract Syntax Tree (AST)
void
generateCode(EDL_AST& ast) {
    generateCode(ast, std::cout);
}
