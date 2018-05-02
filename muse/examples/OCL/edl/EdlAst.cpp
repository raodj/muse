#include "EdlAst.h"
#include <iostream>
#include <algorithm>

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
EDL_AST::addState(const std::string& name) {
    states.push_back(name);
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
    if (std::find(states.begin(), states.end(), name) == states.end()) {
	std::cout << "Invalid state " << name << std::endl;
	return false;  // error
    }
    // Valid transition
    transition.stateChanges.push_back({name, value});
    return true; // success
}

void
EDL_AST::addToExpr(const std::string& token) {
    expr += token;
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
