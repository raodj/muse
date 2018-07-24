#ifndef EDL_AST_CPP
#define EDL_AST_CPP

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

#include "EdlAst.h"
#include <iostream>
#include <algorithm>

Transition::const_iterator
Transition::find(const std::string& comp) const {
    for (Transition::const_iterator curr = cbegin(); (curr != cend()); curr++) {
        if (curr->first == comp) {
            return curr;
        }
    }
    return cend();
}

void
EDL_AST::setName(const std::string& name) {
    epidemicName   = name;
    agentClassName = name;
    agentStateName = name + "State";
}

void
EDL_AST::setAgentClass(const std::string& name) {
    agentClassName = name;
}

void
EDL_AST::setAgentState(const std::string& name) {
    agentStateName = name;
}

/*
void
EDL_AST::addParsedConstant(const boost::fusion::vector2<std::basic_string<char>,
                           double>& info, qi::unused_type, bool& errorFlag) {
    const std::string name  = boost::fusion::at_c<0>(info);
    const double      value = boost::fusion::at_c<1>(info);
    if (constants.find(name) == constants.end()) {
	constants.emplace(name, value);
    } else {
	std::cout << "Duplicate constant definition " << name << std::endl;
	errorFlag = false;
    }
}
*/

bool
EDL_AST::addConstant(const std::string& name, double value) {
    if (constants.find(name) == constants.end()) {
	constants[name] = value;
	return true;  // no errors.
    }
    return false;  // error
}

bool
EDL_AST::addParameter(const std::string& name) {
    parameters.push_back(name);
    return true;
}

bool
EDL_AST::addCompartment(const std::string& name) {
    compartments.push_back(name);
    return true;
}

/*
void
EDL_AST::addStateChange(const boost::fusion::vector2<std::basic_string<char>,
			double>& info, qi::unused_type, bool& errorFlag) {
    const std::string name  = boost::fusion::at_c<0>(info);
    const double      value = boost::fusion::at_c<1>(info);
    if (std::find(states.begin(), states.end(), name) == states.end()) {
	std::cout << "Invalid state " << name << std::endl;
	errorFlag = true;
    } else {
	transition.stateChanges.push_back({name, value});
    }
}
*/

bool
EDL_AST::addStateChange(const std::string& name, double value) {
    if (std::find(compartments.begin(), compartments.end(), name) ==
        compartments.end()) {
	std::cout << "Invalid state " << name << std::endl;
	return false;  // error
    }
    // Valid transition
    transition.push_back(CompChange{name, value});
    return true; // success
}

void
EDL_AST::addToExpr(const std::string& token) {
    // If the token is a compartment, then change its representation
    // to the form comp[token] for code generation later.  Ideally,
    // this should not be done here but have some call into the code
    // generator to get correct conversion.  However, for now, this is
    // acceptable.
    const bool isComp = (std::find(compartments.begin(), compartments.end(),
                                   token) != compartments.end());
    if (isComp) {
        expr += "comp[" + token + "]";
    } else {
        expr += token;
    }
}

void
EDL_AST::addDoubleToExpr(const double val) {
    expr += " ";
    expr += std::to_string(val);
    expr += " ";
}

void
EDL_AST::addCharToExpr(const char op) {
    expr += " ";
    expr += op;
    expr += " ";
}

void
EDL_AST::endTransition(const char dummy) {
    transition.rateExpr = expr;
    transitionList.push_back(transition);
    transition = {};
    expr = "";
}

#endif
