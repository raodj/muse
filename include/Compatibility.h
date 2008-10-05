#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OH.
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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

/** \file Compatibility.h

    \brief Cross-compiler compatibility macros.

    This file contains various macros introduced to ensure cross
    compatibility between various compilers (including: gcc, icc,
    vc++).  Model developers are also encouraged to use these macros
    if they would like to ensure that their models are cross-compiler
    compatible as well.
*/

#include <iostream>

#ifdef _WINDOWS

/** A global not-equal-to function (not an member function).

    This function provides a plug-in replacement for the corresponding
    string comparison function present in Linux version of STL
    implementation but is missing in Windows version. Note that this
    method is defined only under Windows. On Linux the default
    operator!=() provided by basic_string is used instead.
*/
inline bool
operator!=(const std::string& s1, const std::string& s2) {
	return (s1.compare(s2) != 0);
}

/** A global insertion operator for string (not an member function).

    This function provides a plug-in replacement for the corresponding
    insertion operator for std::string present in Linux version of STL
    implementation but is missing in Windows version. Note that this
    method is defined only under Windows. On Linux the default
    operator<<() provided by std::basic_string is used instead.
*/
inline std::ostream&
operator<<(std::ostream& os, const std::string& str) {
	return (os << str.c_str());
}

#endif // _WINDOWS

/** \def UNREFERENCED_PARAMETER Workaround warning C4100 in VS 2005.
    
    This macro is a simple work around to supress the C4100 warning
    (unreferenced parameter).  This warning is generated at Level 4
    under visual studio 2005.  This warning has not been observed
    under gcc (4.2) and therefore on Linux/gcc this macro resolves to
    nothing.
*/
#ifdef _WINDOWS

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(param) param
#endif

#else
#define UNREFERENCED_PARAMETER(param)
#endif

/** \def fmax Macro to return the maximum of 2 values.

    This macro provides a replacement for the fmax() function defined
    in math.h under Linux.  However, VS 2005 does not have this method
    and this macro serves as a replacement.
*/
#ifdef _WINDOWS
#define fmax(x, y) ((x < y) ? y : x)
#endif

/** \def fmin Macro to return the minimum of 2 values.

    This macro provides a replacement for the fmin() function defined
    in math.h under Linux.  However, VS 2005 does not have this method
    and this macro serves as a replacement.
*/
#ifdef _WINDOWS
#define fmin(x, y) ((x < y) ? x : y)
#endif

/** \def vsnprintf_s Macro to define vsnprintf_s if not defined.

    This macro provides a replacement for the vsnprintf_s function
    defined in Windows but is absent in Unix/Linux. This macro simply
    defines vsnprintf_s as vsnprinf in Unix and Linux.
*/
#if (!defined(_WINDOWS) && !defined(vsnprintf_s))
#define vsnprintf_s(buffer, bufSize, count, format, argptr) vsnprintf(buffer, count, format, argptr)
#endif

/** \def _fileno Macro to define _fileno if not defined.

    This macro provides a replacement for the \c _fileno function
    defined in Windows but is absent in Unix/Linux. This macro simply
    defines \c _fileno as \c fileno in Unix and Linux.
*/
#if (!defined(_WINDOWS) && !defined(_fileno))
#define _fileno fileno
#endif

/** \def ctime_s Macro to define ctime_s if not defined.

    This macro provides a replacement for the \c ctime_s function
    defined in Windows but is absent in Unix/Linux. This macro simply
    defines \c ctime_s as \c ctime_r in Unix and Linux.
*/
#if (!defined(_WINDOWS) && !defined(ctime_s))
#define ctime_s(buffer, size, time) ctime_r(time, buffer);
#endif

#endif
