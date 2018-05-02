// #include "edl.h"
#include <unordered_map>
#include <vector>
#include <utility>

// Include boot spirit namespace to reduce number of namespace
// specifications for each Spirit include.
// using namespace boost::spirit;
// namespace phoenix = boost::phoenix;

class Transition {
public:
    std::vector<std::pair<std::string, double>> stateChanges;
    std::string rateExpr;
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

    bool addState(const std::string& name);

    // void addStateChange(const boost::fusion::vector2<std::basic_string<char>,
    // double>& info, qi::unused_type, bool& errorFlag);
    bool addStateChange(const std::string& name, double value);

    void addToExpr(const std::string& token);
    void addCharToExpr(const char op);
    void endTransition(const char dummy);
    
    std::unordered_map<std::string, double> constants;
    std::vector<std::string> parameters;
    std::vector<std::string> states;
    Transition transition;
    std::string expr;
    std::vector<Transition> transitionList;
};
