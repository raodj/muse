#ifndef CONNECTION_THREAD_CPP
#define CONNECTION_THREAD_CPP

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

#include "MUSEGUIApplication.h"
#include "ConnectionThread.h"

ConnectionThread::ConnectionThread(Server &server, QObject *parent) : QThread(parent),
    threadGUI(server) {
    this->server = server;
}

ConnectionThread::~ConnectionThread() {
    delete connection;
}

void
ConnectionThread::run() {

    connection = new SshSocket("Remote Server Operations", NULL,
                               MUSEGUIApplication::getKnownHostsPath(),true, true);
    // Make connections to handle signals
    // Now, intercept the signal to provide the SshSocket with the
    // user's credentials
    connect(connection, SIGNAL(needCredentials(QString*, QString*)),
            &threadGUI, SLOT(interceptRequestForCredentials(QString*, QString*)));

    // Allow prompts for an unknown host to display
    connect(connection->getKnownHosts(), SIGNAL(displayMessageBox(const QString&,
                                                const QString&, const QString&,
                                                const QString&, int*)),
            &threadGUI, SLOT(promptUser(const QString&, const QString&,
                                  const QString&, const QString&, int*)));
    // Allow us to show exception dialog
    connect(this, SIGNAL(exceptionThrown(QString,QString,QString)),
            &threadGUI, SLOT(showException(QString,QString,QString)));

    try {
       connection->connectToHost(server.getName(), NULL, server.getPort());
    }
    catch (const SshException &e) {
        // Format the details
        const QString exceptionDetails =
                e.getErrorDetails().arg(e.getFileName(), QString::number(e.getLineNumber()),
                                 e.getMethodName(), QString::number(e.getSshErrorCode()),
                                 QString::number(e.getNetworkErrorCode()));
        // pass the exception to the threaded connection gui.
        emit exceptionThrown(e.getMessage(), e.getGenericErrorMessage(),
                             exceptionDetails);
    }
}

#endif
