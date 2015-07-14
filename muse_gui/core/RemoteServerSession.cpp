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
#include "RSSAsyncHelper.h"

#include <QObject>
#include <QMessageBox>

#define SUCCESS_CODE 0

RemoteServerSession::RemoteServerSession(const Server& server, QWidget *parent) :
    ServerSession(server, parent), threadGUI{ server },
    socket{ "Remote Server Operations", nullptr,
            MUSEGUIApplication::knownHostsFilePath(),
            true, true }
{
//    socket = NULL;
//    sftpChannel = NULL;
//    sshChannel = NULL;
}

//RemoteServerSession::~RemoteServerSession() {
//    delete sftpChannel;
//    delete sshChannel;
//    delete socket;
//}

void
RemoteServerSession::connectToServer() {
    // Now, intercept the signal to provide the SshSocket with the
    // user's credentials
    connect(&socket, SIGNAL(needCredentials(QString*, QString*)),
            &threadGUI, SLOT(interceptRequestForCredentials(QString*, QString*)));

    auto signal = SIGNAL(displayMessageBox(const QString&, const QString&,
                                           const QString&, const QString&,
                                           int*));

    auto slot = SLOT(promptUser(const QString&, const QString&,
                                const QString&, const QString&, int*));

    // Allow prompts for an unknown host to display
    connect(socket.getKnownHosts(), signal, &threadGUI, slot);

    bool connectResult = socket.connectToHost(server.getName(), nullptr,
                                              server.getPort(), QAbstractSocket::ReadWrite);

    disconnect(socket, SIGNAL(needCredentials(QString*, QString*)),
               &threadGUI, SLOT(interceptRequestForCredentials(QString*, QString*)));
    disconnect(socket.getKnownHosts(), signal, &threadGUI, slot);

    emit connectedToServer(connectResult);

//    auto function = std::bind(&SshSocket::connectToHost, socket, server->getName(), nullptr,
//                              server->getPort(), QAbstractSocket::ReadWrite);

//    RSSAsyncHelper<bool>* test = new RSSAsyncHelper<bool>(&threadedResult, socket,
//                                                          function);
//    socket->changeToThread(test);

//    // Allow us to show exception dialog
//    connect(test, SIGNAL(exceptionThrown(QString,QString,QString)),
//            &threadGUI, SLOT(showException(QString,QString,QString)));

//    test->start();

//    // When the thread has completed, let the caller know the result.
//    connect(test, SIGNAL(finished()), this, SLOT(announceBooleanResult()));

//    // Delete the thread from memory once all of its tasks are complete.
//    connect(test, SIGNAL(finished()), test, SLOT(deleteLater()));
}

//void
//RemoteServerSession::disconnectFromServer() {
//    delete sftpChannel;
//    delete sshChannel;
//    delete socket;

//    sshChannel = NULL;
//    sftpChannel = NULL;
//    socket = NULL;
//}

//int
//RemoteServerSession::exec(const QString &command, QString &stdoutput,
//                          QString &stderrmsgs) {
//    if (!socket) {
//        throw QString{ "Not connected to remote server." };
//    }

//    return SshChannel{ *socket }.exec(command, stdoutput, stderrmsgs);
//}

//int
//RemoteServerSession::exec(const QString &command, QTextEdit &output) {
//    if (!socket) {
//        throw QString{ "Not connected to remote server." };
//    }

//    return SshChannel{ *socket }.exec(command, output);
//}

//bool
//RemoteServerSession::copy(const QString& srcData, const QString &destDirectory,
//                               const QString &destFileName, const int& mode) {
//    if (!socket) {
//        throw QString{ "Not connected to remote server." };
//    }

//    return SshChannel{ *socket }.copy(srcData, destDirectory, destFileName, mode);
//}

//bool
//RemoteServerSession::copy(const QString& destData, const QString &srcDirectory,
//                               const QString &srcFileName) {
//    // Don't try this code if we aren't connected.
//    if (socket == NULL) {
//        throw (std::string) "Not connected to remote server.";
//    }
//    // Check if the channel has been created, if not, create it.
//    if (sshChannel == NULL) {
//        // Might need to try/catch this.
//        sshChannel = new SshChannel(*socket);
//    }
//    bool success = sshChannel->copy(destData, srcDirectory, srcFileName);
//    closeSshChannel();
//    return success;


//}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo RemoteServerSession::fStat(QString *path){

//}

void
RemoteServerSession::getOSType() {
    SshChannel channel{ *socket };

    QString os{ Server::UnknownOS };
    QString out;
    QString err;

    int returnCode = channel.exec("uname -a", out, err);

    if (returnCode != SUCCESS_CODE) {
        out.clear();
        err.clear();

        // Windows specific command 'ver' is equivalent to 'uname -a'
        returnCode = serverSession->exec("ver", out, err);
    }

    if (returnCode == SUCCESS_CODE) {
        if (out.contains(Server::Linux, Qt::CaseInsensitive)) {
            os = Server::Linux;
        } else if (out.contains(Server::Unix, Qt::CaseInsensitive)) {
            os = Server::Unix;
        } else if (out.contains(Server::Windows, Qt::CaseInsensitive)) {
            os = Server::Windows;
        } else if (out.contains(Server::OSX, Qt::CaseInsensitive)) {
            os = Server::OSX;
        }
    }

    emit announceOSType(os);
}

void
RemoteServerSession::mkdir() {
    emit directoryCreated(SFtpChannel{ *socket }.mkdir(directory));
}

void
RemoteServerSession::dirExists() {
    emit directoryExists(SFtpChannel{ *socket }.dirExists(directory));
}

void
RemoteServerSession::createServerData() {
    QString projectsDir{ directory + server->separator() + projectsDirName };
    QString jobsDir{ directory + server->separator() + jobsDirName };
    QString scriptsDir{ directory + server->separator() + scriptsDirName };

    if (!SFtpChannel{ socket }.mkdirs({ projectsDir, jobsDir, scriptsDir })) {
        emit serverDataCreated(false);
        return;
    }

    if (RemoteServerWorkspace{ directory, socket }.save() != "") {
        emit serverDataCreated(false);
        return;
    }

    emit serverDataCreated(true);
}

void
RemoteServerSession::validate() {
    QString projectsDir{ directory + server->separator() + projectsDirName };
    QString jobsDir{ directory + server->separator() + jobsDirName };
    QString scriptsDir{ directory + server->separator() + scriptsDirName };

    if (!SFtpChannel{ socket }.dirsExist({ projectsDir, jobsDir, scriptsDir })) {
        emit directoryValidated(false);
        return;
    }

    if (RemoteServerWorkspace{ directory, socket }.load() != "") {
        emit serverDataCreated(false);
        return;
    }

    emit directoryValidated(true);
}

//void
//RemoteServerSession::setPurpose(const QString &text) {
//    purpose = text;
//}

//SshSocket*
//RemoteServerSession::getSocket() {
//    return socket;
//}

//void
//RemoteServerSession::openSftpChannel() {
//    if (sftpChannel == NULL) {
//        sftpChannel = new SFtpChannel(*socket);
//    }
//}

//void
//RemoteServerSession::closeSftpChannel() {
//    delete sftpChannel;
//    sftpChannel = NULL;
//}

//void
//RemoteServerSession::closeSshChannel() {
//    delete sshChannel;
//    sshChannel = NULL;
//}

//SFtpChannel*
//RemoteServerSession::getSftpChannel() {
//    return sftpChannel;
//}

//void
//RemoteServerSession::announceBooleanResult() {
//    emit booleanResult(threadedResult);
//}

#endif
