#ifndef SERVER_TYPE_PAGE_H
#define SERVER_TYPE_PAGE_H

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
#include <QSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QProgressDialog>
#include "ServerConnectionTester.h"
#include "RemoteServerSession.h"

/**
 * @brief The ServerTypePage class The ServerTypePage presents the user
 * with fields to select the type of server and input information about the
 * server if it is a remote server.
 */
class ServerTypePage : public QWizardPage {
    Q_OBJECT
public:
    ServerTypePage(QWidget *parent = 0);

    /**
     * @brief validatePage The overridden method of QWizardPage. This method
     * spawns a thread that will validate the login credentials and the
     * existence of the remote server entered in this ServerWizardPage.
     * @return Whether or not the ServerWizard can proceed to the next page
     * when the user has clicked the "next" button.
     */
    bool validatePage();

    /**
     * @brief cleanupPage The overridden method of QWizardPage. This method
     * merely ensures that the QProgressDialog is removed from the view if
     * the user selects the "Back" button on this QWizardPage.
     */
    void cleanupPage();

    /**
     * @brief setServerSessionPointer Sets the wizard-wide ServerSession
     * variable so that this page and the ServerInfoPage can operate on the
     * server.
     * @param rss The pointer to the RemoteServerSession
     */
    void setServerSessionPointer(RemoteServerSession* rss);


private slots:
    /**
     * @brief serverTypeChanged Enables or disables the fields on this page
     * that pertain only to a remote server.
     * @param index The index of the currently selected item in
     * serverTypeSelector
     */
    void serverTypeChanged(const int index);

    /**
     * @brief checkConnectionTesterResult Checks the ServerConnectionTester
     * to see if it was successfully able to connect to the server. If so,
     * the wizard should advance to the next page.
     */
    void checkConnectionTesterResult(bool result);


signals:
    void serverSessionCreated(RemoteServerSession* rss);

private:
    QComboBox* serverTypeSelector;
    QLineEdit serverName;
    QLineEdit userId;
    QLineEdit password;
    QSpinBox portNumber;
    QWidget* remoteServerWidget;
    QVBoxLayout* remoteServerLayout;
    ServerConnectionTester* tester;
    QProgressDialog prgDialog;
    bool remoteConnectionVerified;

    RemoteServerSession* remoteServerSession;
    //Server* server;

    /**
     * @brief buildRemoteServerWidget Calls helper methods to assist in the
     * construction of the remoteServerWidget, which is a section of this page
     * that only deals with the remote server.
     */
    void buildRemoteServerWidget();

    /**
     * @brief createServerInfoLayout Creates the server name and port number
     * fields and labels for the remoteServerWidget.
     */
    void createServerInfoLayout();

    /**
     * @brief createCredentialsLayout Creates the user name and password fields
     * and labels for the remoteServerWidget.
     */
    void createCredentialsLayout();

    /**
     * @brief getUserName Gets the user name for the user logged on to the local
     * computer
     * @return The user name.
     */
    QString getUserName();
};

#endif
