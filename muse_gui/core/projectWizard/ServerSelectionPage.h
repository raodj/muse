#ifndef SERVER_SELECTION_PAGE_H
#define SERVER_SELECTION_PAGE_H

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
#include "RemoteServerSession.h"
#include <QComboBox>

/**
 * @brief The ServerSelectionPage class A QWizardPage
 * that prompts the user for the Server that the Project
 * will belong to.
 */
class ServerSelectionPage : public QWizardPage {
    Q_OBJECT
public:
    /**
     * @brief ServerSelectionPage A simple constructor that creates
     * the QComboBox listing of the servers.
     */
    ServerSelectionPage();

public slots:
    /**
     * @brief validatePage Calls checkServerChosen() before advancing
     * to the next page.
     * @return true, always.
     */
    bool validatePage();

signals:
    /**
     * @brief remoteServerSelected Signals to the rest of the ProjectWizard
     * that a remote server was chosen by the user.
     * @param session The session chosen.
     */
    void remoteServerSelected(RemoteServerSession* session);

private slots:
    /**
     * @brief checkServerChosen Checks what type of server was chosen
     * by the user and emits the remoteServerSelected() with a parameter
     * of the session, or NULL if a local server was chosen.
     * @param index The index of the QComboBox that the user has selected.
     */
    void checkServerChosen(int index);

private:
    QComboBox serverList;
    RemoteServerSession* session;
};

#endif // SERVERSELECTIONPAGE_H
