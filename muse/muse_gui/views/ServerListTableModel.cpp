#ifndef SERVER_LIST_TABLE_CPP
#define SERVER_LIST_TABLE_CPP

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

#include "ServerListTableModel.h"
#include "Logger.h"
#include "Core.h"

ServerListTableModel::ServerListTableModel(ServerList& serverList) :
    servers(serverList) {
    //Set the column headers
    setHeaderData(0, Qt::Horizontal, "Server", Qt::DisplayRole);
    setHeaderData(1, Qt::Horizontal, "Status", Qt::DisplayRole);
    setHeaderData(2, Qt::Horizontal, "ID", Qt::DisplayRole);

    connect(&serverList, SIGNAL(serverChanged(ChangeKind,int,int)),
            this, SLOT(handleServerChange(ChangeKind,int,int)));
}

QVariant
ServerListTableModel::headerData(int section, Qt::Orientation orientation,
                                 int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        // Ignore this type of request
        return QVariant();
    }

    // Return strings for column headers
    static const QString ColumnTitles[MAX_COLUMNS] = {"Server", "Status", "ID"};
    return  ColumnTitles[section];
}

QVariant
ServerListTableModel::data(const QModelIndex &index, int role) const {
    if ((index.row() < 0)    || (index.row() >= servers.size()) ||
        (index.column() < 0) || (index.column() >= MAX_COLUMNS)) {
        // A request that cannot be handled.
        return QVariant();
    }

    //Rest of method only executed if role is the display role.
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    //Get the desired row of the server list
    const Server& server = servers.get(index.row());

    //Return the value corresponding to the column.
    switch (index.column()) {
    case 0: return server.getName();
    case 1: return server.getStatus();
    default:
    case 2: return server.getID();
    }
}

void
ServerListTableModel::handleServerChange(ChangeKind change,
                                         int start, int end) {
    switch (change) {
    case ENTRY_INSERTED:
        beginInsertRows(QModelIndex(), start, end);
        endInsertRows();
        break;
    case ENTRY_UPDATED:
        emit dataChanged(createIndex(start, 0, this),
                         createIndex(end, MAX_COLUMNS, this));
        break;
    case ENTRY_DELETED:
        beginRemoveRows(QModelIndex(), start, end);
        endRemoveRows();
    default:
        progLog() << QString("Unhandled ServerList change encountered.")
                  << endl;
        break;
    }
}

#endif
