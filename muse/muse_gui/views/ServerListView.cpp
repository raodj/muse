#ifndef SERVER_LIST_VIEW_CPP
#define SERVER_LIST_VIEW_CPP

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

#include "Core.h"
#include "ServerListView.h"
#include "ServerListTableModel.h"
#include <QHeaderView>

// The constant string that identifies the name of this view
const QString ServerListView::ViewName = "ServerListView";

ServerListView::ServerListView(QWidget *parent) :
    View("ServerListView", parent), serverTable(this) {
    //Show a dotted line grid in the view
    serverTable.setShowGrid(true);
    serverTable.setGridStyle(Qt::DotLine);
    //Stretch the last section across the rest of the view.
    serverTable.horizontalHeader()->setStretchLastSection(true);
    // Set the full row to be selected by default
    serverTable.setSelectionBehavior(QAbstractItemView::SelectRows);
    //This is probably not the way we actually want to implement this.
    serverTable.setModel(new ServerListTableModel());
    // Just a test action to see the toolbar
    addAction(new QAction(QIcon(":/images/16x16/ServerAdd.png"), "Action", 0));
    addAction(new QAction(QIcon(":/images/16x16/ServerConnect.png"), "Action", 0));
    addAction(new QAction(QIcon(":/images/16x16/ServerMyJobs.png"), "Action", 0));
    addAction(new QAction(QIcon(":/images/16x16/ServerInfo.png"), "Action", 0));
    addAction(new QAction(QIcon(":/images/16x16/ServerDelete.png"), "Action", 0));
    // Organize components in this view for dsiplay
    createDefaultLayout(true, &serverTable);
}

#endif
