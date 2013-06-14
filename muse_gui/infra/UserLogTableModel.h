#ifndef USER_LOG_TABLE_MODEL_H
#define USER_LOG_TABLE_MODEL_H

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

#include <QAbstractTableModel>
#include <QTextStream>
#include <QDateTime>
#include "Logger.h"

#define MAX_COLS 3

class UserLog;

class UserLogTableModel : public QAbstractTableModel {
    class UserLogEntry;
    friend QTextStream& operator<<(QTextStream &os, const UserLogEntry& ule);
    friend class UserLog;

    Q_OBJECT

public:
    ~UserLogTableModel() {}
    
    int rowCount(const QModelIndex & = QModelIndex()) const { return logEntries.size(); }
    int columnCount(const QModelIndex & = QModelIndex()) const { return MAX_COLS; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

protected:
    void appendLogEntry(const Logger::LogLevel level,
                        const QMessageLogContext &context,
                        const QString &msg);

private:
    explicit UserLogTableModel(UserLog *parent = 0);

    class UserLogEntry {
    public:
        Logger::LogLevel level;
        QDateTime timestamp;
        QString component;
        QString entry;
    };

    QList<UserLogEntry> logEntries;
};

#endif // USER_LOG_TABLE_MODEL_H
