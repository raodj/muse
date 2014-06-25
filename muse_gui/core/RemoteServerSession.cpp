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
#include <libssh2.h>
#include <libssh2_sftp.h>
#include "RemoteServerSession.h"
#include "MUSEGUIApplication.h"
#include <QObject>
#include <QMessageBox>
//#include "DirectoryASyncHelper.h"
#include "RSSAsyncHelper.h"
#include <QThreadPool>

#define SUCCESS_CODE 0

RemoteServerSession::RemoteServerSession(Server &server, QWidget *parent, QString purpose) :
    ServerSession(server, parent), purpose(purpose), threadGUI(server) {

}

void RemoteServerSession::connectToServer() {
    socket = new SshSocket("Remote Server Operations", NULL, MUSEGUIApplication::getKnownHostsPath(),
                           true, true);

    // Now, intercept the signal to provide the SshSocket with the
    // user's credentials
    connect(socket, SIGNAL(needCredentials(QString*, QString*)),
            &threadGUI, SLOT(interceptRequestForCredentials(QString*, QString*)));

    // Allow prompts for an unknown host to display
    connect(socket->getKnownHosts(), SIGNAL(displayMessageBox(const QString&,
                                                const QString&, const QString&,
                                                const QString&, int*)),
            &threadGUI, SLOT(promptUser(const QString&, const QString&,
                                  const QString&, const QString&, int*)));
    // Allow us to show exception dialog
//    connect(this, SIGNAL(exceptionThrown(QString,QString,QString)),
//            &threadGUI, SLOT(showException(QString,QString,QString)));

    RSSAsyncHelper<bool> test(std::bind(&SshSocket::connectToHost, socket,
                                       server.getName(), NULL, server.getPort(), (QAbstractSocket::ReadWrite) ));

}

void RemoteServerSession::disconnectFromServer() {

}

void RemoteServerSession::getPassword() {

}

int RemoteServerSession::exec(const QString &command, QString &stdoutput,
                              QString &stderrmsgs) {
    Q_UNUSED(stdoutput);
    Q_UNUSED(stderrmsgs);
//    // Create the communication channel
//    LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(connectionThread.getSocket()->getSession());
//    if (channel != NULL) {
//        // Read the streams if the command was executed successfully.
//        if (SUCCESS_CODE == libssh2_channel_exec(
//                    channel, command.toStdString().c_str()) ) {
//            char stdBuffer[0x4000];
//            //int stdOutSize = libssh2_channel_read_ex(channel, 0, stdBuffer, sizeof(stdBuffer));
//            libssh2_channel_read_ex(channel, 0, stdBuffer, sizeof(stdBuffer));
//            stdoutput = stdBuffer;
//        }



    //}


}

int RemoteServerSession::exec(const QString &command, QTextDocument &output) {

    Q_UNUSED(command);
    Q_UNUSED(output);
}

bool RemoteServerSession::verifyServerHostKey(const QString &hostName, const int port,
                                              const QString &serverHostKeyAlgorithm,
                                              const char &serverHostKey) {
    Q_UNUSED(hostName);
    Q_UNUSED(port);
    Q_UNUSED(serverHostKey);
    Q_UNUSED(serverHostKeyAlgorithm);

}

void RemoteServerSession::addKnownHost(const QString &hostName, const int port,
                                       const QString &serverHostKeyAlgorithm,
                                       const char &serverHostKey) {
    Q_UNUSED(hostName);
    Q_UNUSED(port);
    Q_UNUSED(serverHostKeyAlgorithm);
    Q_UNUSED(serverHostKey);

}

void RemoteServerSession::loadKnownHosts() {

}

void RemoteServerSession::copy(std::istream &srcData, const QString &destDirectory,
                               const QString &destFileName, const QString &mode) {
    Q_UNUSED(srcData);
    Q_UNUSED(destDirectory);
    Q_UNUSED(destFileName);
    Q_UNUSED(mode);

}

void RemoteServerSession::copy(std::ostream &destData, const QString &srcDirectory,
                               const QString &srcFileName) {
    Q_UNUSED(destData);
    Q_UNUSED(srcDirectory);
    Q_UNUSED(srcFileName);


}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path){

//}

void
RemoteServerSession::mkdir(const QString &directory) {
    /*
    // Create an sftp variable
    LIBSSH2_SFTP* sftpSession = NULL;    
    // Attempt to set it up
    sftpSession = libssh2_sftp_init(connectionThread.getSocket()->getSession());
    // Check for success
    if (sftpSession != NULL) {
        int returnCode = libssh2_sftp_mkdir(sftpSession, directory.toStdString().c_str(), 0700);
        QMessageBox msgBox;
        if (libssh2_sftp_shutdown(sftpSession) != SUCCESS_CODE ||
                returnCode != SUCCESS_CODE) {
            msgBox.setText("There was an issue creating the install directory.<br/>"\
                           "This likely means that the directory currently exists," \
                           " which is not good. Please change the install directory.");
            msgBox.exec();
        }
        // Announce the result of mkdir
        emit directoryCreated(returnCode == SUCCESS_CODE);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Failed to connect.");
        msgBox.exec();
        emit directoryCreated(false);
    }
    */
//    DirectoryASyncHelper* helper = new DirectoryASyncHelper("mkdir", directory, connectionThread.getSocket());
//    connect(helper, SIGNAL(result(bool)), this, SLOT(promptUserIfMkdirFailed(bool)));
//    connect(helper, SIGNAL(result(bool)), this, SIGNAL(directoryCreated(bool)));
//    QThreadPool::globalInstance()->start(helper);
}

void RemoteServerSession::rmdir(const QString &directory) {
 /*
    // Create an sftp variable
    LIBSSH2_SFTP* sftpSession = NULL;
    // Attempt to set it up
    sftpSession = libssh2_sftp_init(connectionThread.getSocket()->getSession());
    // Check for success
    if (sftpSession != NULL) {
        int returnCode = libssh2_sftp_rmdir(sftpSession, directory.toStdString().c_str());
        QMessageBox msgBox;
        if (libssh2_sftp_shutdown(sftpSession) != SUCCESS_CODE ||
                returnCode != SUCCESS_CODE) {
            msgBox.setText("Couldn't close sftp");
            msgBox.exec();
        }
        // Anounce the result of rmdir
        emit directoryRemoved(returnCode == SUCCESS_CODE);
    }
    else {
        QMessageBox msgBox;
        msgBox.setText("Failed to connect.");
        msgBox.exec();
    }
    */
//    DirectoryASyncHelper* helper = new DirectoryASyncHelper("rmdir", directory, connectionThread.getSocket());
//    connect(helper, SIGNAL(result(bool)), this, SLOT(promptUserIfRmdirFailed(bool)));
//    connect(helper, SIGNAL(result(bool)), this, SIGNAL(directoryRemoved(bool)));
//    QThreadPool::globalInstance()->start(helper);
}

void RemoteServerSession::setPurpose(const QString &text) {
    Q_UNUSED(text);

}


void
RemoteServerSession::promptUserIfMkdirFailed(const bool result) {
    if (!result) {
        QMessageBox msgBox;
        msgBox.setText("There was an issue creating the install directory.<br/>"\
                       "This likely means that the directory currently exists," \
                       " which is not good. Please change the install directory.");
        msgBox.exec();
    }
}

void
RemoteServerSession::promptUserIfRmdirFailed(const bool result) {
    if (!result) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't remove the directory.");
        msgBox.exec();
    }
}

#endif
