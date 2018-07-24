#ifndef AGENT_GENERATOR_H
#define AGENT_GENERATOR_H

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
#include "Generator.h"

class AgentGenerator : public Generator {
public:
    /** A default constructor.

        The default constructor merely initializes the instance
        variables to default initial values.
    */
    AgentGenerator() {}

    /** The destructor.

        The destructor does not have any special tasks to perform and
        is present merely to adhere to coding conventions.
     */
    ~AgentGenerator() {}

    /** The top-level code generation method.

        This method is the primary interface method for code
        generation.  It uses the supplied ast and generates C++ code
        that uses MUSE's heterogeneous computing macros.

        \param[in] ast The abstract syntax tree to be used to generate
        a C++ simulation.

        \param[out] os The output stream to where the data is to be
        written.
    */
    void generateAgent(const EDL_AST& ast);
    
protected:

    /** Helper method to generate the ODE for a given compartment.

        This method is an internal helper method that is used to
        generate 1 ODE associated with a given compartment.  This
        method walks the list of transitions and uses those
        transitions dealing with the given compartment to generate the
        ODE.

        \param[in] ast The AST to be used for code generation.

        \param[in] comp The name of the compartment for which ODE is
        to be generated.

        \param[out] os The output stream to where the ODE is to be
        written.
    */
    void generateODEs(const EDL_AST& ast, const std::string& comp,
                      std::ostream& os) const;

    /** Convenience method to generate the auto generating 'N' for sum
        of population in all the compartments.

        This method is simple and generates an equation of the form "N
        = comp[s] + comp[i] + comp[r];"

        \param[in] ast The AST to be used for code generation.

        \param[in] var The resulting var to be generated.  Typically
        this is just 'N'

        \param[out] os The otuput stream to where the equation is to
        be written.

        \param[in] indent An initial indentation string to be used.
    */
    void generateTotalPop(const EDL_AST& ast, const std::string& var,
                          std::ostream& os,
                          const std::string indent = "    ") const;

    /** Generate the ode() method that contains all the ODEs for
        different compartments in this model.

        This method is the top-level ODE generation method that
        generates the ode() method with different equations for all
        the compartments.  This method uses other helper methods to
        generate the ODEs.

        \param[in] ast The AST to be used for code generation.

        \param[os] os The output stream to where the code is to be
        written.
    */
    void generateODEs(const EDL_AST& ast, std::ostream& os) const;

    /** Get the list of compartment names to be used to generate a
        C/C++ enumeration data type.

        \param[in] ast The AST from where the list of compartments are
        to be obtained.

        \param[in] delim The delimiter to be used at the end of each
        compartment.

        \param[in] indent Any additional indentation to be used.
    */
    std::string getCompartmentList(const EDL_AST& ast,
                                   const std::string delim = ",\n",
                                   const std::string indent = "") const;

    /** Generate the enumeration data type for the compartments used
        in EDL.

        This method generates a top-level enum data type in the form
        "enum CompartmentNames { s, e, i, r, INVALID};"

        \param[in] edl The AST to be used for generating the transitions.

        \param[out] os The otuput stream to where the equation is to
        be written.        
    */
    void generateCompartments(const EDL_AST& ast,
                              std::ostream& os) const;
    
    /** The SSA transitions to generate for a given equation.

        This method generates a comma-separated list of changes for
        the different compartments associated with a given
        transitions.  This method uses the getCompartmentList method
        to generate entries for each transition.

        \param[in] edl The AST to be used for generating the transitions.

        \param[in] tr The transition from where the state changes are
        to be generated.

        \return A string containing the list of changes to different
        compartments for the given transition.
    */
    std::string getCompTransitions(const EDL_AST& edl,
                                   const Transition& tr) const;

    /** Method to generate the SSA matrix used by an SSA method.


        \param[in] ast The AST to be used for code generation.
        
        \param[out] os The otuput stream to where the equation is to
        be written.

        \param[in] indent The indentation to be used. 
     */
    void generateSSAMatrix(const EDL_AST& ast, std::ostream& os,
                           const std::string& indent = "    ") const;

    /** Method to generate the getSSARates method that provides the
        SSA rates required by the SSA algorithms in MUSE.

        \param[in] ast The AST to be used for code generation.
        
        \param[out] os The otuput stream to where the equation is to
        be written.
        
        \param[in] indent The indentation to be used.       
    */
    void generateSSARates(const EDL_AST& ast, std::ostream& os,
                          const std::string& indent = "    ") const;

    /** Helper method to generate the list of named constants used in
       the EDL description.

        \param[in] ast The AST to be used for code generation.
        
        \param[out] os The otuput stream to where the equation is to
        be written.
        
        \param[in] indent The indentation to be used.
    */
    void generateConstants(const EDL_AST& ast, std::ostream& os,
                           const std::string& indent = "    ") const;

    /** Helper method to generate the agent's header using the
        AgentHeaderTemplate.

        \param[in] ast The AST to be used for generating the header
        file.
    */
    void generateAgentHeader(const EDL_AST& ast) const;
    
    /** Helper method to generate the agent's source file using the
        AgentSourceTemplate.

        \param[in] ast The AST to be used for generating the header
        file.
    */
    void generateAgentSource(const EDL_AST& ast) const;

private:
    /** A predefined string constant that holds the template for an
        agent's header file. This string is initialized by directly
        including StateTemplate.h into the source code using raw
        string literal support introduced in C++11.  This string is
        used to generate the header from a given EDL AST.
    */
    static const std::string AgentHeaderTemplate;

    /** A predefined string constant that holds the template for an
        agent's source file. This string is initialized by directly
        including StateTemplate.h into the source code using raw
        string literal support introduced in C++11.  This string is
        used to generate the source file for an agent from a given EDL
        AST.
    */
    static const std::string AgentSourceTemplate;
};

#endif
