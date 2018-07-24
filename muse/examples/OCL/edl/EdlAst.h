#ifndef EDL_AST_H
#define EDL_AST_H

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

#include <unordered_map>
#include <vector>
#include <utility>

// Include boot spirit namespace to reduce number of namespace
// specifications for each Spirit include.
// using namespace boost::spirit;
// namespace phoenix = boost::phoenix;

// A change in a compartment is represented as a string (for
// compartment name) and the change (positive/negative) to the
// compartment.
using CompChange = std::pair<std::string, double>;

class Transition :  public std::vector<CompChange> {
public:
    std::string rateExpr;
    Transition::const_iterator find(const std::string& comp) const;
};

class EDL_AST {
public:
    std::string epidemicName;
    std::string agentClassName;
    std::string agentStateName;
    
    void setName(const std::string& name);
    void setAgentClass(const std::string& name);
    void setAgentState(const std::string& name);


    // void addParsedConstant(const boost::fusion::vector2<std::basic_string<char>,
    //                        double>& info, qi::unused_type, bool& errorFlag);
    bool addConstant(const std::string& name, double value);

    bool addParameter(const std::string& name);

    bool addCompartment(const std::string& name);

    // void addStateChange(const boost::fusion::vector2<std::basic_string<char>,
    // double>& info, qi::unused_type, bool& errorFlag);
    bool addStateChange(const std::string& name, double value);

    void addToExpr(const std::string& token);
    void addDoubleToExpr(const double token);
    void addCharToExpr(const char op);
    void endTransition(const char dummy);
    
    std::unordered_map<std::string, double> constants;
    std::vector<std::string> parameters;
    std::vector<std::string> compartments;
    Transition transition;
    std::string expr;
    std::vector<Transition> transitionList;
};

#endif
