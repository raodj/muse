#ifndef MAKEFILE_GENERATOR_H
#define MAKEFILE_GENERATOR_H

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

/** Generate a makefile for compiling the generated executable.

    This class is used by the EDL code generator to generate a
    standard Makefile compiling the generated source code.  This class
    essentially uses a pre-set Makefile template, replaces pertinent
    string constants, to generate the final Makefile.
*/
class MakefileGenerator : public Generator {
public:
    /** A default constructor.

        The default constructor merely initializes the instance
        variables to default initial values.
    */
    MakefileGenerator() {}

    /** The destructor.

        The destructor does not have any special tasks to perform and
        is present merely to adhere to coding conventions.
     */
    ~MakefileGenerator() {}

    /** The top-level code generation method.

        This method is the primary interface method for code
        generation.  It uses the supplied ast and generates a Makefile
        for compling the source code produced by other generators.

        \param[in] ast The abstract syntax tree to be used to generate
        the Makefile.
    */
    void generateMakefile(const EDL_AST& ast);

private:
    /** A predefined string constant that holds a template to be used
        for generating the Makefile. This string is initialized by
        directly including MakefileTemplate.inc into the source code
        using raw string literal support introduced in C++11.
    */
    static const std::string MakefileTemplate;
};

#endif
