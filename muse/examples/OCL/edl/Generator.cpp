#ifndef GENERATOR_CPP
#define GENERATOR_CPP

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

#include "Generator.h"

std::string
Generator::replace(const std::string& source, const std::string& search,
                   const std::string& replace, size_t startPos) const {
    std::string retVal = source;
    while (startPos != std::string::npos) {
        // Find the next occurrence of search string.
        if ((startPos = retVal.find(search, startPos)) != std::string::npos) {
            // Found an occurrence of search string. Replace it.
            retVal.replace(startPos, search.size(), replace);
            startPos += replace.size();
        }
    }
    return retVal;
}

#endif
