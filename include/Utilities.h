#ifndef UTILITIES_H
#define UTILITIES_H

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

#include <time.h>
#include <cstring>

/** \def ASSERT(x)

    \brief Define a convenient macro for using c asserts.

    Define a custom macro ASSERT (note the all caps) method to be used
    instead of the c \c assert method to enable compiling ASSERT
    usages in and out of the source code.  When the compiler flag \c
    DEVELOPER_ASSERTIONS is specified then the ASSERT call defaults to
    the normal \c assert method.  For example ASSERT (\<<em>boolean
    expression</em>\>)} will be mapped to \c assert (\<<em>boolean
    expression</em>\>).  If the compiler flag \c DEVELOPER_ASSERTIONS
    is not specified then the ASSERT simply gets compiled out.
*/
#ifndef ASSERT
#ifdef DEVELOPER_ASSERTIONS
#include <assert.h>

#define ASSERT(x) assert(x)

#else // !DEVELOPER_ASSERTIONS

#define ASSERT(x)

#endif
#endif

/** \def DEBUG(x)

    \brief Define a convenient macro for conditionally compiling
    additional debugging information.

    Define a custom macro DEBUG (note the all caps) macro to be used
    to conditionally compile in debugging code to generate detailed
    logs.  This helps to minimize code modification to insert and
    remove debugging messages.
*/
#ifndef DEBUG
#ifdef DEBUG_OUTPUT

#define DEBUG(x) x

#else // !DEBUG_OUTPUT

#define DEBUG(x)

#endif
#endif

/**\def END_NAMESPACE(x)

   \brief A convenient macro for ending namespace declarations

   This macro provides a convenient approach for defining namespaces
   in C++ headers and source files.  Using these macros ensures that
   indendation for namespaces do not clutter or eat up screen
   real-estate.  This macro should be used in conjunction with the
   BEGIN_NAMESPACE macro.
   
   \see BEGIN_NAMESPACE
*/
#define END_NAMESPACE(x) }

/**\def BEGIN_NAMESPACE(x)

   \brief A convenient macro for generating namespace declarations

   This macro provides a convenient approach for defining namespaces
   in C++ headers and source files.  Using this macro ensures that
   indendation for namespaces do not clutter or eat up screen
   real-estate.  Here is an example of how to use this macro:

   \code

   #include "Utilities.h"

   BEGIN_NAMESPACE(muse);

   // Rest of the C++ class definitions etc. go here.
   
   END_NAMESPACE(muse);
   
   \endcode

   \see END_NAMESPACE
*/
#define BEGIN_NAMESPACE(x) namespace x {

/** \def getTimeStamp
    
    \brief Get's the file modification timestamp for a given file
    name.
    
    This method provides a portable (to Windows and Linux/Unix)
    implementation for a helper method to obtain the modification
    timestamp for a given file.
    
    \param[in] fileName The file name (with full path) for which the
    modification time stamp is desired.  If the fileName is NULL then
    this method simply returns the buffer without any modifications.
    
    \param[out] buffer The buffer into which the time stamp is to be
    written.
    
    \return A pointer to the buffer.
*/
char* getTimeStamp(const char *fileName, char *buffer);

/** \def getSystemTime
    
    \brief Returns the string representation of the supplied time data
    structure.
    
    This method provides a portable (to Windows and Linux/Unix)
    implementation for a helper method to obtain the string
    representation of a given encoded time_t datastructure.
    
    \param[out] buffer The buffer into which the string representation
    of the supplied time is to be written.  This pointer must be
    capable of holding at least 128 characters.  If this pointer is
    NULL, then this method exits immediately.

    \param[in] codedTime The encoded time_t data structure to be
    converted to a string representation.  If this parameter is NULL,
    then the current system time is converted to a string and filled
    into the supplied buffer.

    \return The pointer to the buffer passed in.
*/
char* getSystemTime(char *buffer, const time_t *codedTime = NULL);

/** \def ctime_s Macro to define ctime_s if not defined.

    This macro provides a replacement for the \c ctime_s function
	defined in Windows but is absent in Unix/Linux. This macro 
    simply defines \c ctime_s as \c ctime_r in Unix and Linux.
*/
#if (!defined(_WINDOWS) && !defined(ctime_s))
#define ctime_s(buffer, size, time) ctime_r(time, buffer);
#endif

#endif
