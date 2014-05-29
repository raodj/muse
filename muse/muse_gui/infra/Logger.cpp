#ifndef LOGGER_CPP
#define LOGGER_CPP

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "Logger.h"
#include "ProgrammerLog.h"
#include "UserLog.h"

QString
Logger::toString(const LogLevel level) {
    static const QString LevelNames[] = {"VERBOSE", "NOTICE", "WARNING", "ERROR"};
    return LevelNames[(int) level];
}

int Logger::toInt(const QString &level){
    static const QString LevelNames[] = {"VERBOSE", "NOTICE", "WARNING", "ERROR"};
    for(int i=0; i<sizeof(LevelNames); i++)
        if(level.contains(LevelNames[i]))
            return i;
    return -1;
}

QDebug
Logger::log(const char *formatStr, ...) const {
    if (formatStr != NULL) {
        va_list varArgList;
        va_start(varArgList, formatStr); // use variable arg list
        // Process the variadic arguments first.
        logMsg = QString().vsprintf(formatStr, varArgList);
        va_end(varArgList);
    }
    // Construct the debug stream to add additional messages
    // to the variadiac logs
    return QDebug(&logMsg);
}


QDebug
Logger::log(const LogLevel level, const char *formatStr, ...) const {
    // Update the level
    this->logLevel = level;
    // Process any format string and parameters specified.
    if (formatStr != NULL) {
        va_list varArgList;
        va_start(varArgList, formatStr); // use variable arg list
        // Process the variadic arguments first.
        logMsg = QString().vsprintf(formatStr, varArgList);
        va_end(varArgList);
    }
    // Construct the debug stream to add additional messages
    // to the variadiac logs
    return QDebug(&logMsg);
}

Logger::~Logger() {
    if (!isUserLogEntry) {
        ProgrammerLog::programmerLog.appendLogEntry(logLevel, context, logMsg);
    } else {
        UserLog::globalUserLog.appendLogEntry(logLevel, context, logMsg);
    }
}

#endif // LOGGER_CPP
