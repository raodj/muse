#ifndef WORKSPACE_SELECT_PAGE_H
#define WORKSPACE_SELECT_PAGE_H

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

#include <QWizardPage>
#include <QComboBox>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <vector>

class WorkspaceSelectPage : public QWizardPage {
    Q_OBJECT
public:
    WorkspaceSelectPage(QWidget *parent = 0);

    void setWorkspaceOptions(std::vector<QString> options);

private slots:
    /**
     * @brief createNewWorkspace Ask the user where they want the new workspace
     * to be located and attempt to create it
     *
     * This method will be invoked when the newWorkspaceButton button is pressed
     */
    void createNewWorkspace();

    /**
     * @brief workspaceSelected Use the currently selected workspace
     *
     * This method will be invoked when the selectWorkspaceButton button is
     * pressed
     */
    void workspaceSelected();

private:
    QVBoxLayout layout;
    QComboBox workspaceSelector;
    QPushButton selectWorkspaceButton;
    QPushButton newWorkspaceButton;

    static const QString workspaceExistsMessage;
};

#endif // WORKSPACE_SELECT_PAGE_H
