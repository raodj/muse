#ifndef GENERATOR_H
#define GENERATOR_H

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

#include <string>
#include "EdlAst.h"

/** Base class that just provides some common utility methods.

    This is a common base class used by all the code generators.  This
    class just provides some common helper methods and utilities that
    are used by different code generators.
*/
class Generator {
public:
    /** The destructor.

        The destructor does not have any special tasks to perform and
        is present merely to enable polymorphism.
     */
    virtual ~Generator() {}

    /** Convenience method to replace all occurrences of search string
        with a replace string.

        This is a relatively straightforward method that replaces all
        ocurrences of search in a given source string with a given
        replacement string.  For example replace("This is a test",
        "is", "**") returns the string "Th** ** a test".

        \param[in] source The source string.
        
        \param[in] search The string to search for.

        \param[in] replace The

        \param[in] startPos An optional starting index in source
        string from where the replacement operations should commence.
     */
    std::string replace(const std::string& source, const std::string& search,
                        const std::string& replace, size_t startPos = 0) const;
    
protected:
    /** A default constructor.

        The default constructor is protected to ensure that this class
        is not directly instantiated.  Instead, instantiate one of the
        derived class members.
    */    
    Generator() {}
};

#endif
