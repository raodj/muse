#ifndef THREADED_CONNECTION_GUI_H
#define THREADED_CONNECTION_GUI_H

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

#include <QWidget>
#include <QWaitCondition>
#include <QMutex>
#include "Server.h"
#include "SshSocket.h"


class ThreadedConnectionGUI : public QObject {
Q_OBJECT
public:
    /**
     * @brief ThreadedConnectionGUI Default constructor.
     * This is used only for the purpose of using this class'
     * slots when a server does not exist.
     *
     * @see ServerConnectionTester is the main case for the
     * use of this constructor.
     */
    ThreadedConnectionGUI();

    /**
     * @brief ThreadedConnectionGUI The primary constructor.
     * This constructor is normally the one used when a
     * RemoteServerSession is created. This allows MUSE
     * to display GUI prompts/issues when connecting to the
     * remote server.
     * @param server The server being connected to.
     */
    ThreadedConnectionGUI(Server* server);
    ~ThreadedConnectionGUI();
    ThreadedConnectionGUI(const ThreadedConnectionGUI& tcg);

    // For controlling the threads
    static QWaitCondition userHasResponded, passUserData;
    static QMutex mutex, userDataMutex;

    Server* getServer();

public slots:
    /**
     * @brief interceptRequestForCredentials Intercepts the the SshSocket's
     * request for credentials to prevent the LoginCredentials dialog from
     * appearing on the screen.
     * @param username The pointer to the SshSocket's username.
     * @param passWord The pointer to the SshSocket's password.
     */
    void interceptRequestForCredentials(QString *username, QString *passWord);

    /**
     * @brief promptUser Displays user prompts from the threaded SSH connection
     * to allow the SSH class to perform its connection correctly.
     * @param windowTitle The window title of the QMessageBox.
     * @param text The primary text of the QMessageBox.
     * @param informativeText The informative text for the QMessageBox.
     * @param detailedText The detailed text for the QMessageBox.
     * @param userChoice The button pressed to close the QMessageBox.
     */
     void promptUser(const QString &windowTitle, const QString& text,
                    const QString &informativeText,
                    const QString &detailedText, int* userChoice);

    /**
     * @brief showException Shows a dialog explaining the exception thrown
     * by the SshSocket.
     * @param message The primary message to display.
     * @param genErrorMessage The descriptive message to display.
     * @param exceptionDetails The exact details of the error.
     */
     void showException(const QString& message, const QString& genErrorMessage,
                       const QString& exceptionDetails);

     /**
      * @brief showMessage Shows a simple message to the user that only contains
      * the primary text option of QMessageBox.
      * @param message The message to display.
      */
     void showMessage(const QString& message);

signals:


private:
     Server* server;
};

#endif
