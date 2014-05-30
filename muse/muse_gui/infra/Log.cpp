#ifndef LOG_CPP
#define LOG_CPP

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

#include "Log.h"
#include <QMessageBox>
Log::Log() {
    // Nothing else to be done for now.
}

Log::~Log() {
    // Empty constructor begets an empty destructor.
}

bool
Log::setLogFileName(const QString& fileName) {
    // Close the existing file (if any) underlying the log stream.
    logStream.setDevice(NULL);
    logFile.close();

    // Setup the new file name specified by the user.
    logFile.setFileName(fileName);
    emit logFileNameUpdated();

    // Open the log file for appending logs.
    if (!logFile.open(QFile::Append | QFile::Text)) {
        // Error opening log file for appending. Report an error and bail out.
        QString msg = "Error opening log file ('" + fileName +
                "') for appending: " + logFile.errorString();
        emit errorSavingLog(msg);
        return false; // error!
    }
    // Setup the new file as the destination for streamling logs.
    logStream.setDevice(&logFile);
    // Check to ensure the log stream is still good.
    checkLogStream();
    // Return overall status
    return (logStream.status() == QTextStream::Ok);
}


QString
Log::getLogFileName() {
    return logFile.fileName();
}

// Convenience method to check if the log stream is in working condition. If not
// it reports an error.
void
Log::checkLogStream() {
    if (logStream.status() != QTextStream::Ok) {
        QString msg = "Error appending to log file ('" + getLogFileName() +
                "') - " + logFile.errorString() + ". Logging to file stopped.";
        emit errorSavingLog(msg);
    }
}

void
Log::setSaveStatus(bool saveToFile) {
    saveLogs = saveToFile;
    emit saveStatusChanged(saveLogs);
}

void
Log::toggleSaveStatus() {
    setSaveStatus(!saveLogs);
}

#endif // LOG_CPP
