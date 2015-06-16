#ifndef SERVER_LIST_VIEW_H
#define SERVER_LIST_VIEW_H

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

#include "View.h"
#include "ServerListTableModel.h"

#include <QTableView>

#include <memory>

/**
 * @brief The ServerListView class Provides a visual, tabular listing of
 * servers that the user has connected to in the past.
 */
class ServerListView : public View {
    Q_OBJECT
public:
    ServerListView(QWidget* parent = 0);

    /**
     * @brief ViewName A constant string to consistently refer to the name
     * of this view. This string is set to "ServerListView".
     */
    static const QString ViewName;

public slots:
    void updateView();

protected slots:
    /**
     * @brief showServerWizard Shows the ServerWizard when the user selects
     * to add a server connection.
     */
    void showServerWizard();

private:
    QTableView serverTable;
    QAction* addServerButton;
    QAction* connectToServerButton;
    QAction* myJobsButton;
    QAction* serverInfoButton;
    QAction* deleteServerButton;

    std::unique_ptr<ServerListTableModel> tableModel;

    /**
     * @brief initializeToolBarButtons Initializes and adds the the buttons
     * to the toolbar for the server list view.
     */
    void initializeToolBarButtons();

};

#endif
