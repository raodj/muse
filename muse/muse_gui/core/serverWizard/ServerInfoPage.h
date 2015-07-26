#ifndef SERVER_INFO_PAGE_H
#define SERVER_INFO_PAGE_H

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
#include <QTextEdit>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QProgressDialog>
#include "RemoteServerSession.h"
#include "LocalServerSession.h"

/**
 * @brief The ServerInfoPage class The ServerInfoPage presents the user
 * with prompts for their description of the server, the install
 * directory, and polling delay.
 */
class ServerInfoPage : public QWizardPage {
    Q_OBJECT
public:
    /**
     * @brief ServerInfoPage The default constructor for the ServerInfoPage.
     * The constructor creates the layout for the page and places the
     * components of the page into the layouts accordingly.
     * @param parent
     */
    ServerInfoPage(QWidget* parent = 0);

    /**
     * @brief initializePage Overridden method of QWizardPage to verify
     * whether or not the browse button should be enabled by checking
     * if the user selected to create a local or remote host, and to
     * auto-populate the QLineEdit that displays the default install
     * directory for the server.
     */
    void initializePage();



public slots:
    /**
     * @brief validatePage The overridden method of QWizardPage. This method
     * spawns a thread that will validate the install directory entered
     * by the user.
     * @return Whether or not the ServerWizard can proceed to the next page
     * when the user has clicked the "next" button.
     */
    bool validatePage();

    /**
     * @brief setServerSessionPointer Sets the wizard-wide ServerSession
     * variable so that this page and the ServerSummaryPage can operate on the
     * server.
     * @param rss The pointer to the RemoteServerSession
     */
    void setServerSessionPointer(std::shared_ptr<ServerSession> ss);

    /**
     * @brief getMkdirResult Gets the result of calling mkdir
     * on the ServerSession to verify the install directory.
     * This methods sets mkdirSucceeded = result.
     * @param result
     */
    void getMkdirResult(bool result);

    /**
     * @brief getRmdirResult Gets the result of calling rmdir
     * on the LocalServerSession to verify the install directory.
     * This methods sets installDirectoryVerified = result.
     * @param result
     */
    //void getRmdirResult(bool result);

    /**
     * @brief getDirExistsResult Gets the result of calling dirExists()
     * on the ServerSession to verify if the install directory exists.
     *
     * @param result True if the dir exists, false otherwise
     */
    void getDirExistsResult(bool result);

    /**
     * @brief getServerDataCreatedResult Gets the result of calling
     * createServerData() on the ServerSession to verify if the required
     * data was successfully created.
     *
     * @param result True if the data was created, false otherwise
     */
    void getServerDataCreatedResult(bool result);

    /**
     * @brief getDirValidatedResult Gets the result of calling
     * validate() on the ServerSession to verify if the selected directory
     * is a valid Server.
     *
     * @param result True if the dir is a valid Server, false otherwise
     */
    void getDirValidatedResult(bool result);

private:
    QTextEdit serverDescription;
    QLineEdit installDirectoryDisplay;
    QPushButton browse;
    QSpinBox pollingDelay;
    QProgressDialog prgDialog;

    static const QString InstallDirectoryMessage;

    std::shared_ptr<ServerSession> serverSession;

    bool installDirChecked;
    bool installDirExists;
    bool installDirHasServerData;
    bool installDirValidated;

private slots:
    /**
     * @brief browseFileSystem Spawns a QFileDialog for the user
     * to select the install directory for MUSE.
     */
    void browseFileSystem();
};

#endif
