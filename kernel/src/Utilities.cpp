#ifndef UTILITIES_CPP
#define UTILITIES_CPP

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

#include "Utilities.h"
#include <sys/types.h>
#include <sys/stat.h>

char*
getTimeStamp(const char *fileName, char *buffer) {
    if (fileName == NULL) {
        // Nothing further to be done here.
        return buffer;
    }
    // The follwing structure will contain the file information.
    struct stat fileInfo;
    int returnValue = 0;
#ifdef _WINDOWS
    returnValue = _stat(fileName, &fileInfo);
#else
    // The only difference between windows and Linux is the extra "_"
    // at the beginning of stat() method.
    returnValue = stat(fileName, &fileInfo);
#endif
    // Check to ensure there were no errors.  If there were errors
    // exit immediately.
    if (returnValue == -1) {
        // O!o! there was an error.
        return buffer;
    }
    // Convert the last modification time to string and return it back
    // to the caller.
    return getSystemTime(buffer, &fileInfo.st_mtime);
}

char* getSystemTime(char *buffer, const time_t *encodedTime) {
    if (buffer == NULL) {
        // Nothing more to do.
        return NULL;
    }
    // Get instant system time.
    time_t timeToConv = time(NULL);
    if (encodedTime != NULL) {
        // If we have a valid time supplied, then override system time.
        timeToConv = *encodedTime;
    }
    // Convert the time.
    ctime_s(buffer, 128, &timeToConv);
    // Return the buffer back to the caller
    return buffer;
}

#endif
