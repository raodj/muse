#ifndef REMOTE_SERVER_SESSION_CPP
#define REMOTE_SERVER_SESSION_CPP

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
#include "RemoteServerSession.h"
#include "MUSEGUIApplication.h"
#include "SFtpChannel.h"
#include "RemoteServerWorkspace.h"

//#include <libssh2.h>
//#include <libssh2_sftp.h>

#include <QObject>
#include <QMessageBox>

#define SUCCESS_CODE 0

RemoteServerSession::RemoteServerSession(Server* server, QWidget *parent,
                                         QString purpose) :
    ServerSession(server, parent), purpose(purpose), threadGUI(server) {
    socket = NULL;
    sftpChannel = NULL;
    sshChannel = NULL;
}

RemoteServerSession::~RemoteServerSession() {
    delete sftpChannel;
    delete sshChannel;
    delete socket;
}

void
RemoteServerSession::connectToServer() {
    socket = new SshSocket("Remote Server Operations", NULL,
                           MUSEGUIApplication::knownHostsFilePath(),
                           true, true);

    // Now, intercept the signal to provide the SshSocket with the
    // user's credentials
    connect(socket, SIGNAL(needCredentials(QString*, QString*)),
            &threadGUI, SLOT(interceptRequestForCredentials(QString*, QString*)));

    auto signal = SIGNAL(displayMessageBox(const QString&, const QString&,
                                           const QString&, const QString&,
                                           int*));

    auto slot = SLOT(promptUser(const QString&, const QString&,
                                const QString&, const QString&, int*));

    // Allow prompts for an unknown host to display
    connect(socket->getKnownHosts(), signal, &threadGUI, slot);

    auto function = std::bind(&SshSocket::connectToHost, socket, server->getName(), nullptr,
                              server->getPort(), QAbstractSocket::ReadWrite);

    RSSAsyncHelper<bool>* test = new RSSAsyncHelper<bool>(&threadedResult, socket,
                                                          function);
    socket->changeToThread(test);

    // Allow us to show exception dialog
    connect(test, SIGNAL(exceptionThrown(QString,QString,QString)),
            &threadGUI, SLOT(showException(QString,QString,QString)));

    test->start();

    // When the thread has completed, let the caller know the result.
    connect(test, SIGNAL(finished()), this, SLOT(announceBooleanResult()));

    // Delete the thread from memory once all of its tasks are complete.
    connect(test, SIGNAL(finished()), test, SLOT(deleteLater()));
}

void
RemoteServerSession::disconnectFromServer() {
    delete sftpChannel;
    delete sshChannel;
    delete socket;

    sshChannel = NULL;
    sftpChannel = NULL;
    socket = NULL;
}

int
RemoteServerSession::exec(const QString &command, QString &stdoutput,
                          QString &stderrmsgs) {
    // Don't try this code if we aren't connected.
    if (socket == NULL) {
        throw (std::string) "Not connected to remote server.";
    }
    // Check if the channel has been created, if not, create it.
    if (sshChannel == NULL) {
        // Might need to try/catch this.
        sshChannel = new SshChannel(*socket);
    }

    int retCode = sshChannel->exec(command, stdoutput, stderrmsgs);
    closeSshChannel();
    return retCode;
}

int
RemoteServerSession::exec(const QString &command, QTextEdit &output) {
    // Don't try this code if we aren't connected.
    if (socket == NULL) {
        throw (std::string) "Not connected to remote server.";
    }
    // Check if the channel has been created, if not, create it.
    if (sshChannel == NULL) {
        // Might need to try/catch this.
        sshChannel = new SshChannel(*socket);
    }
    int retCode = sshChannel->exec(command, output);
    closeSshChannel();
    return retCode;
}

bool
RemoteServerSession::copy(const QString& srcData, const QString &destDirectory,
                               const QString &destFileName, const int& mode) {
    // Don't try this code if we aren't connected.
    if (socket == NULL) {
        throw (std::string) "Not connected to remote server.";
    }
    // Check if the channel has been created, if not, create it.
    if (sshChannel == NULL) {
        // Might need to try/catch this.
        sshChannel = new SshChannel(*socket);
    }
    bool success = sshChannel->copy(srcData, destDirectory, destFileName, mode);
    closeSshChannel();
    return success;
}

bool
RemoteServerSession::copy(const QString& destData, const QString &srcDirectory,
                               const QString &srcFileName) {
    // Don't try this code if we aren't connected.
    if (socket == NULL) {
        throw (std::string) "Not connected to remote server.";
    }
    // Check if the channel has been created, if not, create it.
    if (sshChannel == NULL) {
        // Might need to try/catch this.
        sshChannel = new SshChannel(*socket);
    }
    bool success = sshChannel->copy(destData, srcDirectory, srcFileName);
    closeSshChannel();
    return success;


}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo RemoteServerSession::fStat(QString *path){

//}

void
RemoteServerSession::mkdir() {
    if (sftpChannel == nullptr) {
        sftpChannel = new SFtpChannel(*socket);
    }

    emit directoryCreated(sftpChannel->mkdir(directory));
}

void
RemoteServerSession::dirExists() {
    if (sftpChannel == nullptr) {
        sftpChannel = new SFtpChannel(*socket);
    }

    emit directoryExists(sftpChannel->dirExists(directory));
}

void
RemoteServerSession::createServerData() {
    if (sftpChannel == nullptr) {
        sftpChannel = new SFtpChannel(*socket);
    }

    QString projectsDir{ directory + server->separator() + projectsDirName };
    QString jobsDir{ directory + server->separator() + jobsDirName };
    QString scriptsDir{ directory + server->separator() + scriptsDirName };

    if (!sftpChannel->mkdirs({ projectsDir, jobsDir, scriptsDir })) {
        emit serverDataCreated(false);
        return;
    }

    if (RemoteServerWorkspace{ directory }.save() != "") {
        emit serverDataCreated(false);
        return;
    }

    emit serverDataCreated(true);
}

void
RemoteServerSession::validate() {
    if (sftpChannel == nullptr) {
        sftpChannel = new SFtpChannel(*socket);
    }

    QString projectsDir{ directory + server->separator() + projectsDirName };
    QString jobsDir{ directory + server->separator() + jobsDirName };
    QString scriptsDir{ directory + server->separator() + scriptsDirName };

    if (!sftpChannel->dirsExist({ projectsDir, jobsDir, scriptsDir })) {
        emit directoryValidated(false);
        return;
    }

    if (RemoteServerWorkspace{ directory }.load() != "") {
        emit serverDataCreated(false);
        return;
    }

    emit directoryValidated(true);
}

void
RemoteServerSession::setPurpose(const QString &text) {
    purpose = text;
}

SshSocket*
RemoteServerSession::getSocket() {
    return socket;
}

void
RemoteServerSession::openSftpChannel() {
    if (sftpChannel == NULL) {
        sftpChannel = new SFtpChannel(*socket);
    }
}

void
RemoteServerSession::closeSftpChannel() {
    delete sftpChannel;
    sftpChannel = NULL;
}

void
RemoteServerSession::closeSshChannel() {
    delete sshChannel;
    sshChannel = NULL;
}

SFtpChannel*
RemoteServerSession::getSftpChannel() {
    return sftpChannel;
}

void
RemoteServerSession::announceBooleanResult() {
    emit booleanResult(threadedResult);
}

#endif
