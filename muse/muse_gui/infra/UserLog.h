#ifndef USER_LOG_H
#define USER_LOG_H

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
#include "UserLogTableModel.h"

class UserLog : public Log {
    Q_OBJECT
    friend class Logger;
public:
    static UserLog& get() { return globalUserLog; }
    Logger::LogLevel getLevel(const int row) const;


    UserLogTableModel& getEntries() { return logData; }

signals:
    /**
     * @brief saveNewestEntry Signals that the user has elected to save the log to a file,
     * and will save the newest entry to the file as long as the entry has a level greater
     * than or equal to what the user has set in the log filter.
     */
    void saveNewestEntry();

public slots:

    /**
     * @brief write Writes this UserLog to a file using the
     * logStream instance variable as the desired stream.
     *
     * @param lowestLevelToShow The lowest severity level
     * of log entries to be written to the file.
     *
     * /note This parameter comes from the index of the
     * QComboBox containing the listing of log severity levels.
     * This means that the QComboBox MUST maintain the same order
     * of severity levels as the Logger enumerations.
     */
    void writeAllEntries(const int lowestLevelToShow);

    /**
     * @brief writeLastEntry Writes the last (newest) log entry
     * to the logFile as specified by the user.
     *
     * @param lowestLevelToShow The lowest severity level
     * of log entries to be written to the file.
     *
     * /note This parameter comes from the index of the
     * QComboBox containing the listing of log severity levels.
     * This means that the QComboBox MUST maintain the same order
     * of severity levels as the Logger enumerations.
     */
    void writeLastEntry(const int lowestLevelToShow);

protected slots:
    void appendLogEntry(const Logger::LogLevel level,
                        const QMessageLogContext &context,
                        const QString &msg);



protected:
    static UserLog globalUserLog;

private:
    UserLog();
    ~UserLog();
    UserLogTableModel logData;

};

#endif // USER_LOG_H
