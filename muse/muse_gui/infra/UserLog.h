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

    void write(QTextStream& os);
    UserLogTableModel& getEntries() { return logData; }

public slots:

    /**
     * @brief Saves the log to a file. Only the logs that meet the selected
     * importance filter will be saved to the file.
     * @param level The lowest importance level the user wishes to have saved
     * to the log file.
     */
    void saveLog(QTextStream &os, const QString &level);

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
