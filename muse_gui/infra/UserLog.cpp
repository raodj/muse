#ifndef USER_LOG_CPP
#define USER_LOG_CPP

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

#include "UserLog.h"
#include <QFile>

// The global singleton instance of programmer log
UserLog UserLog::globalUserLog;

UserLog::UserLog() : logData(NULL) {
}

UserLog::~UserLog() {
    // Empty constructor implies empty destructor
}

Logger::LogLevel
UserLog::getLevel(const int row) const {
    if ((row < 0) || (row >= logData.logEntries.size())) {
        return Logger::LOG_ERROR;
    }
    return logData.logEntries[row].level;
}

void
UserLog::write(QTextStream& os) {
    for(int i = 0; ((os.status() == QTextStream::Ok) &&
                    (i < logData.logEntries.size())); i++) {
        os << logData.logEntries[i] << endl;
    }
    os.flush();
}

void
UserLog::appendLogEntry(const Logger::LogLevel level,
                        const QMessageLogContext &context, const QString &msg) {
    // Save data in our logs
    logData.appendLogEntry(level, context, msg);
    // Let views (if any) know the model/data has changed
    emit logChanged();
    // Stream the log entry to the log file (if it has been setup).
    if (isLogStreamGood()) {
        const int lastRow = logData.logEntries.size() - 1;
        logStream << logData.logEntries[lastRow];
        logStream.flush();  // Ensure data is flushed out to detect errors early.
        // Ensure the data was successfuly written and report any errors.
        checkLogStream();
    }
}

#endif
