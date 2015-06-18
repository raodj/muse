#ifndef PROJECT_LIST_TABLE_MODEL_H
#define PROJECT_LIST_TABLE_MODEL_H

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
#include "Project.h"
#include "Server.h"

//#define MAX_COLUMNS 2
static const int MAX_COLUMNS = 2;

class ProjectListItem {
public:
    ProjectListItem(Project ip, Server is) {}
        // : project(ip), server(is) {}

    Project project;
    Server server;
};

class ProjectListTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    ProjectListTableModel();
    int rowCount(const QModelIndex & = QModelIndex()) const { return projectEntries.size(); }
    int columnCount(const QModelIndex & = QModelIndex()) const { return MAX_COLUMNS; }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    static ProjectListTableModel& model;

public slots:
    /**
     * @brief appendProjectEntry Adds a project to this ProjectListTableModel.
     * @param project The project to add
     */
    void appendProjectEntry(Project& project, Server& server);

signals:
    /**
     * @brief serverAdded Alerts any views using this ServerListTableModel
     * that a server has been added to the table and that the views should
     * be updated.
     */
    void selectedServerChanged();
    void projectAdded();

private:
    QList<ProjectListItem> projectEntries;
};

#endif
