#ifndef SIMULATION_GENERATOR_H
#define SIMULATION_GENERATOR_H

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
#include "Generator.h"

/** Generate the top-level simulation header and source files.

    This class is used by the EDL code generator to generate the
    definition for the simulation class associated with an agent.
    This class essentially uses a pre-set template, replaces pertinent
    string constants, to generate the simulation class.
*/
class SimulationGenerator : public Generator {
public:
    /** A default constructor.

        The default constructor merely initializes the instance
        variables to default initial values.
    */
    SimulationGenerator() {}

    /** The destructor.

        The destructor does not have any special tasks to perform and
        is present merely to adhere to coding conventions.
     */
    ~SimulationGenerator() {}

    /** The top-level code generation method to generate support
        files.

        This method is used to generate 3 fixed files, namely:
        OclEqnSolvers.c, OclEqnSolvers.c_inc, and OclEqnSolvers.h
        (file names are fixed as well). These files are used by the
        generated simulation for solving ODE/SSA equations in the
        model.  These are fixed files and do not require any variable
        susbstitutions.
    */
    void generateEqnSolvers();   

    /** The top-level code generation method.

        This method is the primary interface method for code
        generation.  It uses the supplied ast and generates C++ code
        that uses MUSE's heterogeneous computing macros.

        \param[in] ast The abstract syntax tree to be used to generate
        a C++ simulation.

        \param[out] os The output stream to where the data is to be
        written.

        \return The name of the state class generated by this method.
    */
    void generateSim(const EDL_AST& ast);

protected:
    /** Convenience method to replace several variables in a given
        template.

        This is a convenience method use dto replace several variables
        in both header and source templates.

        \param[in] srcTemplate The source template in which variables
        are to be substituted.

        \return The rsulting string after all substitutions have been
        applied.
    */
    std::string replaceAll(const EDL_AST& ast,
                           const std::string& srcTemplate) const;
    
private:
    /** A predefined string constant that holds the template for an
        simulation's header file. This string is initialized by
        directly including SimulationTemplate.h into the source code
        using raw string literal support introduced in C++11.  This
        string is used to generate the header from a given EDL AST.
    */
    static const std::string SimulationHeaderTemplate;

    /** A predefined string constant that holds the template for an
        simulation's source file. This string is initialized by
        directly including SimulationTemplate.cpp into the source code
        using raw string literal support introduced in C++11.  This
        string is used to generate the source file for an agent from a
        given EDL AST.
    */
    static const std::string SimulationSourceTemplate;

    /** A predefined string constant that holds the header-file code
        for OpenCL compatible ODE & SSA equation solvers. This string
        is initialized by directly including OclEqnSolvers.h.inc into
        the source code using raw string literal support introduced in
        C++11.  This string is used to generate the OclEqnSolvers.h in
        the generated simulation for an epidemic from a given EDL AST.
    */
    static const std::string OclEqnSolversHeader;
    
    /** A predefined string constant that holds the source code for
        OpenCL compatible ODE & SSA equation solvers. This string is
        initialized by directly including OclEqnSolvers.c.inc into the
        source code using raw string literal support introduced in
        C++11.  This string is used to generate the OclEqnSolvers.c in
        the generated simulation for an epidemic from a given EDL AST.
    */
    static const std::string OclEqnSolversSource;
};

#endif
