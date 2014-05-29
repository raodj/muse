#ifndef PROGRAMMER_LOG_H
#define PROGRAMMER_LOG_H

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

class ProgrammerLog : public Log {
    Q_OBJECT
    friend class Logger;
public:
    static ProgrammerLog& get() { return programmerLog; }
    const QString& getEntries() const { return logEntries; }
    void write(QTextStream& os);
public slots:
    /**
     * @brief Savesthe log to a file. In this case, the write() has
     * the same functionality,so the write() is called from saveLog().
     * If no use for write() is discovered, then this method will
     * replace write() in the API.
     *
     * @param os The output stream to save the file.
     */
    void saveLog(QTextStream &os);
protected slots:
    void appendLogEntry(const Logger::LogLevel,
                        const QMessageLogContext &context,
                        const QString &msg);

protected:
    QString logEntries;
    static ProgrammerLog programmerLog;

private:
    ProgrammerLog();
    ~ProgrammerLog();
};

#endif // PROGRAMMER_LOG_H
