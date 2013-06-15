#ifndef USER_LOG_TABLE_MODEL_CPP
#define USER_LOG_TABLE_MODEL_CPP

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

#include "UserLogTableModel.h"
#include "UserLog.h"
#include <QIcon>

// A named constant for tab (to make code a bit more readable)
#define TAB "\t"

UserLogTableModel::UserLogTableModel(UserLog *parent) :
    QAbstractTableModel(parent) {
    // Nothing else to be done here for now
}

QTextStream&
operator<<(QTextStream &os, const UserLogTableModel::UserLogEntry& ule) {
    os << Logger::toString(ule.level) << TAB
       << ule.timestamp.toString()    << TAB
       << ule.component               << TAB
       << ule.entry;
    return os;
}

QVariant
UserLogTableModel::data(const QModelIndex &index, int role) const {
    if ((index.row() < 0)    || (index.row() >= logEntries.size()) ||
        (index.column() < 0) || (index.column() >= MAX_COLS)) {
        // A request that cannot be handled.
        return QVariant();
    }
    // Obtain corresponding row of user logs
    const UserLogEntry &ule = logEntries[index.row()];

    // Display role is used only for the first column to include
    // an icon for log-level value.
    if ((role == Qt::DecorationRole) && (index.column() == 0)) {
        switch (ule.level)
        {
        case Logger::LOG_VERBOSE:   return QIcon(":/images/32x32/log_verbose.png");
        case Logger::LOG_NOTICE:    return QIcon(":/images/32x32/log_notice.png");
        case Logger::LOG_WARNING:   return QIcon(":/images/32x32/log_warning.png");
        case Logger::LOG_ERROR:     return QIcon(":/images/32x32/log_error.png");
        }

        return QIcon(":/images/32x32/log_verbose.png");
    }
    // Rest of the code works only if role is display role.
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    // Return suitable value (role is display role).
    switch (index.column()) {
    case 0: return ule.timestamp.toString();
    case 1: return ule.component;
    default:
    case 2: return ule.entry;
    }
}

QVariant
UserLogTableModel::headerData(int section, Qt::Orientation orientation,
                              int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        // Ignore this type of request
        return QVariant();
    }
    // Return strings for column headers
    static const QString ColumnTitles[MAX_COLS] =
    {"Timestamp", "Component", "Log Message"};
    return  ColumnTitles[section];
}

void
UserLogTableModel::appendLogEntry(const Logger::LogLevel level,
                                  const QMessageLogContext &context,
                                  const QString &msg) {
    // Let base class know a row is being inserted/appended
    beginInsertRows(QModelIndex(), logEntries.size(),
                    logEntries.size() + 1);
    // Create and populate a new UserLogEntry object
    UserLogEntry ule;
    ule.timestamp = QDateTime::currentDateTime();
    ule.level     = level;
    ule.component = context.category;
    ule.entry     = msg;
    // Add it to the list of entries in this model
    logEntries.append(ule);
    // Let the base class know the insertion is done.
    this->endInsertRows();
}

#endif
