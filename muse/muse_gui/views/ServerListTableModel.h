#ifndef SERVER_LIST_TABLE_MODEL_H
#define SERVER_LIST_TABLE_MODEL_H

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
#include <QList>

#include "Server.h"
#include "ServerList.h"

/**
 * @brief The ServerListTableModel class The table model for showing a
 * listing of servers in MUSE GUI. This class provides the interface
 * between the view and the in-memory storage classes in workspace. This
 * is purely an adapter class and does not (and should not) contain any
 * information. Instead it obtains the necessary information from the
 * workspace entries.
 */
class ServerListTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    /**
     * @brief ServerListTableModel Constructor to create a server list
     * table model.
     *
     * @param serverList The underlying workspace server list to be used
     * to obtain the actual data.
     */
    ServerListTableModel(ServerList& serverList);

    /**
     * @brief rowCount Override base class method to return number of entries.
     *
     * @return This method returns number of server entries (in the workspace).
     */
    int rowCount(const QModelIndex & = QModelIndex()) const {
        return servers.size();
    }

    /**
     * @brief columnCount Override base class method to return number of
     * columns supported by this model.
     *
     * This model currently supports a fixed number of 3 columns.
     *
     * @return The number of columns supported by this method.
     */
    int columnCount(const QModelIndex & = QModelIndex()) const { return MAX_COLUMNS; }

    /**
     * @brief data Overrides base class method to return the actual data for
     * a given row and column.
     *
     * @param index The row and column information for which the data is to
     * be returned.
     *
     * @param role The role for which data is desired.
     *
     * @return The value (if any) associated with the given index and role.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /**
     * @brief headerData Override base class interface method to return the
     * column title information.
     *
     * @param section The column (or section) for which the title is to be
     * returned.
     *
     * @param orientation The orientation value is currently ignored by this
     * method.
     *
     * @param role The display role for which the title is to be returned.
     * This value is currently ignored.
     *
     * @return The column title for the given section.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected slots:
    /**
     * @brief handleServerChange Slot to notify view(s) about change in
     * server information.
     *
     * @param change The type of change encountered by the server.
     *
     * @param start The starting index position in the server list where
     * the change occurred.
     *
     * @param end The ending index position in the server list where
     * the change ends.
     */
    void handleServerChange(ChangeKind change, int start, int end);

private:
    /**
     * @brief MAX_COLUMNS The maximum number of columns supported by
     * this server model.
     */
    static const int MAX_COLUMNS = 3;

    /**
     * @brief servers Reference to the server list to be used by this
     * model.
     */
    ServerList& servers;

};

#endif
