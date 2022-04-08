/** Example of Epidemic Description Language (EDL):

# Lines starting with pound sign are comments
epidemic chikv(ChikVSEIR, ChikVState) {
    constants {
       temp = 10.2;   # Temperature
    }
    parameters {
       beta, mu, psi, omega;  # These are parameters passed in to the kernel
    }
    compartments {
        s, e, i, r;  # These are compartments and are part of the state.
    }
    transitions {
       s += 1 @ n * mu;
       s -= 1, e += 1 @ s * beta;
       i += 1, e -= 1 @ e * psi;
       e -= 1, r += 1 @ i * omega; 
    }
}

*/

#include <unordered_map>
#include <algorithm>
#include <utility>
#include <functional>
#include "edl_parser.h"
#include "EdlAst.h"
#include "SimulationGenerator.h"

// Include boot spirit namespace to reduce number of namespace
// specifications for each Spirit include.
using namespace boost::spirit;
namespace phoenix = boost::phoenix;

// Helper macros to make the grammer rules below more readable.
#define CALL_AST(func) std::bind(&EDL_AST::func, &ast, std::placeholders::_1)
#define CALL_AST3(func) std::bind(&EDL_AST::func, &ast, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

#define CALL_AST_STR_DBL(func) std::bind(&edl_grammar::callAstMethod, this, &EDL_AST::func, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

#define CALL_GRAMMR(func) std::bind(&edl_grammar::func, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)

void param2(const boost::fusion::vector2<std::basic_string<char>, double>& info) {
    std::cout << boost::fusion::at_c<0>(info) << "="
              << boost::fusion::at_c<1>(info) << std::endl;
}

template <typename Iterator>
struct pl0_skipper : public qi::grammar<Iterator> {
        
    pl0_skipper() : pl0_skipper::base_type(skip, "PL/0") {
        skip = ascii::space | ('#' >> *(qi::char_ - qi::eol) >> qi::eol);
    }
    qi::rule<Iterator> skip;
};


template <typename Iterator, typename Skipper = pl0_skipper<Iterator> >
struct edl_grammar : qi::grammar<Iterator, Skipper> {

    using SkipperType = pl0_skipper<Iterator>;
    using AstStrDbl = std::function<bool(EDL_AST*, const std::string&, double)>;
    
    void handleConstant(const boost::fusion::vector2<std::basic_string<char>,
			double>& info, qi::unused_type, bool& errorFlag) {
    }

    void callAstMethod(AstStrDbl func,
                       const boost::fusion::vector2<std::basic_string<char>,
                       double>& info, qi::unused_type, bool& errorFlag) {
	const std::string name  = boost::fusion::at_c<0>(info);
	const double      value = boost::fusion::at_c<1>(info);
	errorFlag = func(&ast, name, value);
    }
    
    edl_grammar() : edl_grammar::base_type(epidemic, "edl")   {
        token = qi::alpha >> *(qi::alnum | '_');
        token.name("token");

        optionalClassStateNames = -( '('
                                     > (
                                        token[CALL_AST(setAgentClass)] ||
                                        (',' > token[CALL_AST(setAgentState)] )
                                        )
                                     > ')'
                                   );
        optionalClassStateNames.name("class-state-names");

        constant = ( token
                     > '='
                     > double_
                     > ';'
                     ) [ CALL_AST_STR_DBL(addConstant) ];


            
        optionalConstants = -( lit("constants")
                               > '{'
                               > *(constant)
                               > '}'
                             );
        optionalConstants.name("constants");

        parameterList = ( token [ CALL_AST(addParameter) ]
                          > *(',' > token [ CALL_AST(addParameter) ] )
                          > ';'
                          );
        optionalParameters = -( lit("parameters")
                                > '{'
                                > *(parameterList)
                                > '}'
                                );
	optionalParameters.name("parameters");
	
	// Grammar for processing a list of states.
        compList = ( token [ CALL_AST(addCompartment) ]
		      > *(',' > token [ CALL_AST(addCompartment) ] )
		      > ';'
		      );
        optionalCompartments = -( lit("compartments")
                                  > '{'
                                  > *(compList)
                                  > '}'
                                  );
	optionalCompartments.name("compartments");

	// Grammar for processing a set of transitions
	stateChange = ( token
			> lit("+=")
			> double_
			)  [ CALL_AST_STR_DBL(addStateChange) ];
	stateChange.name("state-change");

	// Grammar rules for processing expressions in transitions
	expr = (term
		>> *( ( qi::char_('+') [CALL_AST(addCharToExpr)] >> term ) |
		      ( qi::char_('-') [CALL_AST(addCharToExpr)] >> term ) )
		);
	expr.name("expr");
	
	term = exprGroup
	    >> *( ( qi::char_('*') [CALL_AST(addCharToExpr)] >> exprGroup ) |
		  ( qi::char_('/') [CALL_AST(addCharToExpr)] >> exprGroup )
		  );
	term.name("term");
	
	exprGroup = ( double_ [CALL_AST(addDoubleToExpr)]
                      | ( -( qi::char_('-') [CALL_AST(addCharToExpr)] ) >> token [CALL_AST(addToExpr)])
		      | ( qi::char_('(') [CALL_AST(addCharToExpr)]
			  >> expr
			  >> qi::char_(')') [CALL_AST(addCharToExpr)]
			  )
		      );
	exprGroup.name("exprGroup");
	
	transition = ( stateChange
		       > *(lit(",") > stateChange)
		       > '@'
		       > expr
		       > qi::char_(';') [CALL_AST(endTransition)]
		       );
	transition.name("transition");
	
	transitionList = ( lit("transitions")
			   > '{'
			   > *(transition)
			   > '}'
			   );
	transitionList.name("transitionList");

        epidemic = lit("epidemic")
            > token [CALL_AST(setName)]
            > optionalClassStateNames
            > '{'
            > optionalConstants
            > optionalParameters
	    > optionalCompartments
	    > transitionList
            > '}'
            > qi::eoi;
        epidemic.name("epidemic");

	// skipper = qi::space; // *lit('#');
        // skipper.name("skipper");
        // start = qi::skip(skipper) [ epidemic ];
        // start.name("edl");
        setErrorHandling();
        debug(start);
        // debug(skipper);
    }

    void setErrorHandling() {
        qi::on_error<qi::fail>(epidemic,
                               std::cout << phoenix::val("Error! Expecting ")
                                         << qi::_4 << phoenix::val(" here: \"")
                                         << phoenix::construct<std::string>(qi::_3, qi::_2)
                                         << phoenix::val("\"") << std::endl);
    }
    EDL_AST ast;
    qi::rule<Iterator> start;
    pl0_skipper<Iterator> skipper;
    qi::rule<Iterator, SkipperType> optionalClassStateNames;
    qi::rule<Iterator, SkipperType> constant;    
    qi::rule<Iterator, SkipperType> optionalConstants;
    // Rules for parsing parameters
    qi::rule<Iterator, SkipperType> parameterList;
    qi::rule<Iterator, SkipperType> optionalParameters;
    // Rules for parsing state
    qi::rule<Iterator, SkipperType> compList;
    qi::rule<Iterator, SkipperType> optionalCompartments;
    // Rules for parsing transitions
    qi::rule<Iterator, SkipperType> stateChange;
    qi::rule<Iterator, SkipperType> transition;
    // The following rules are handling math expressions in transitions
    qi::rule<Iterator, SkipperType> expr;
    qi::rule<Iterator, SkipperType> term;
    qi::rule<Iterator, SkipperType> exprGroup;

    qi::rule<Iterator, SkipperType> transitionList;
    // Top-level grammar which is a collection of rules.
    qi::rule<Iterator, SkipperType> epidemic;
    qi::rule<Iterator, std::string(), SkipperType> token, cpu, ocl;
};

template <typename Iterator>
bool parseEDL(Iterator first, Iterator last, EDL_AST& ast) {
    using namespace boost::spirit;
    edl_grammar<Iterator> grammar;
    pl0_skipper<Iterator> skipper;
    bool result = qi::phrase_parse(first, last, grammar, skipper);
    if (first != last) {
        // fail if we did not get a full match
        return false;
    }
    // Move over the ast
    ast = std::move(grammar.ast);
    return result;
}

bool parseEDL(const std::string& edlFilePath, EDL_AST& ast) {
    std::ifstream edlSrcFile(edlFilePath);
    if (!edlSrcFile.good()) {
        std::cerr << "Error opening EDL file: " << edlFilePath << std::endl;
        return false;
    }
    // Ensure white spaces are preserved for parser
    edlSrcFile.unsetf(std::ios::skipws);
    // wrap istream into iterators
    boost::spirit::istream_iterator edl(edlSrcFile), eof;
    // Let the parser parse it now (template parameter deduced as
    // iterators are of the same type below).
    return parseEDL(edl, eof, ast);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Specify EDL file to process.\n";
        return 1;
    }
    EDL_AST ast;
    if (!parseEDL(argv[1], ast)) {
        std::cout << "Error parsing EDL.\n";
        return 1;
    }
    // Now that the EDL is parsed into an Abstract Syntax Tree (AST),
    // we can now use the AST to generate code.
    SimulationGenerator gen;
    gen.generateSim(ast);
    return 0;
}
