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
#include "MUSEGUIApplication.h"
#include <QProgressDialog>
#include "SSHKnownHosts.h"

ServerConnectionTester::ServerConnectionTester(QString userName,
                                               QString password,
                                               QString hostName,
                                               const int portNumber,
                                               QWidget* mainThread,
                                               QObject *parent) :
    QThread(parent) {
    // Default success status
    success = false;
    this->userName = userName;
    this->password = password;
    this->hostName = hostName;
    this->portNumber = portNumber;
    ptrToMainThread = mainThread;


}

QWidget*
ServerConnectionTester::getParentWidget() {
    return ptrToMainThread;
}

void
ServerConnectionTester::run() {

    SSHKnownHosts sshkh(MUSEGUIApplication::getKnownHostsPath());
    connection = new SshSocket("Testing connection", ptrToMainThread,
                               MUSEGUIApplication::getKnownHostsPath());

    // Disconnect the signal for credentials to avoid the popup dialog.
    connection->disconnect(connection, SIGNAL(needCredentials(SshSocket&,
                                                        const libssh2_knownhost*, QString&, QString&, bool&)),
                                                  connection, SLOT(getCredentials(SshSocket&,
                                                        const libssh2_knownhost*, QString&, QString&, bool&)));

    // Now, intercept the signal to provide the SshSocket with the
    // user's credentials
    connect(connection, SIGNAL(needCredentials(SshSocket&,
                                                           const libssh2_knownhost*,
                                                           QString&,QString&,bool&)), this,
                        SLOT(interceptRequestForCredentials(SshSocket&,
                                                            const libssh2_knownhost*,
                                                            QString&,QString&,bool&)));
    // Pass along the user's credentials
    connect(this, SIGNAL(passUserCredentials(SshSocket&,
                                             const libssh2_knownhost*,
                                             QString&,QString&,bool&)),
            connection, SLOT(getCredentials(SshSocket&,
                                            const libssh2_knownhost*,
                                            QString&,QString&,bool&)));


    // Okay, time to test the connection
    //QProgressDialog* prgBar = new QProgressDialog();
    //prgBar->show();

    success = connection->connectToHost(hostName);
    //exec();

    delete connection;
}

void
ServerConnectionTester::interceptRequestForCredentials(SshSocket &ssh, const libssh2_knownhost *hostInfo,
                                                       QString &userID, QString &password, bool &cancel) {
    userID = userName;
    password = this->password;

    emit passUserCredentials(ssh, hostInfo, userID, password, cancel);

}


bool
ServerConnectionTester::getResult() {
    return success;
}

#endif
