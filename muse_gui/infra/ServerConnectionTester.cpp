#ifndef SERVER_CONNECTION_TESTER_CPP
#define SERVER_CONNECTION_TESTER_CPP

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

#include "ServerConnectionTester.h"
#include "ThreadedConnectionGUI.h"
#include "MUSEGUIApplication.h"
#include "Workspace.h"

#include <QProgressDialog>
#include <QMessageBox>

ServerConnectionTester::ServerConnectionTester(QString userName,
                                               QString password,
                                               QString hostName,
                                               const int portNumber,
                                               QObject *parent) :
    QThread(parent) {

    // Default success status
    success = false;
    this->userName = userName;
    this->password = password;
    this->hostName = hostName;
    this->portNumber = portNumber;

}

void
ServerConnectionTester::run() {
    connection = new SshSocket("Testing connection", NULL,
                               MUSEGUIApplication::knownHostsFilePath(), true, true);

    // Now, intercept the signal to provide the SshSocket with the
    // user's credentials
    connect(connection, SIGNAL(needCredentials(QString*, QString*)),
            this, SLOT(interceptRequestForCredentials(QString*, QString*)));


    // Allow prompts for an unknown host to display
    connect(connection->getKnownHosts(), SIGNAL(displayMessageBox(const QString&,
                                                const QString&, const QString&,
                                                const QString&, int*)),
            &tcg, SLOT(promptUser(const QString&, const QString&,
                                  const QString&, const QString&, int*)));
    // Allow us to show exception dialog
    connect(this, SIGNAL(exceptionThrown(QString,QString,QString)),
            &tcg, SLOT(showException(QString,QString,QString)));
    // Okay, time to test the connection
    try {
        success = connection->connectToHost(hostName);
    }
    catch (const SshException &e) {
        const QString exceptionDetails =
                e.getErrorDetails().arg(e.getFileName(), QString::number(e.getLineNumber()),
                                 e.getMethodName(), QString::number(e.getSshErrorCode()),
                                 QString::number(e.getNetworkErrorCode()));
        emit exceptionThrown(e.getMessage(), e.getGenericErrorMessage(),
                             exceptionDetails);
    }
    delete connection;
}

void
ServerConnectionTester::interceptRequestForCredentials(QString* username, QString* passWord) {

    // Prevent other threads from accessing this data.
    ThreadedConnectionGUI::userDataMutex.lock();
    // Change the username credential to the username input in the wizard
    *username = userName;
    // Change the password credential to the password input in the wizard
    *passWord = password;
    // Let the background thread continue
    ThreadedConnectionGUI::passUserData.wakeAll();
    // Release the lock on the data
    ThreadedConnectionGUI::userDataMutex.unlock();
}


bool
ServerConnectionTester::getResult() {
    return success;
}



#endif
