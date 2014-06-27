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
#include "SFtpChannel.h"
#include "RSSAsyncHelper.h"



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


    RSSAsyncHelper<bool>* test = new RSSAsyncHelper<bool>(&threadedResult, socket, std::bind(&SshSocket::connectToHost, socket,
                                        server.getName(),
                                        (QProgressDialog *) NULL,
                                        server.getPort(),
                                        QAbstractSocket::ReadWrite));
    socket->changeToThread(test);

    test->start();
    // When the thread has completed, let the caller know the result.
    connect(test, SIGNAL(finished()), this, SLOT(announceBooleanResult()));
    // Delete the thread from memory once all of its tasks are complete.
    connect(test, SIGNAL(finished()), test, SLOT(deleteLater()));


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
    // We need an SftpChannel to run mkdir
    SFtpChannel mkdirChannel(*socket);
    // Make an AsyncHelper
    RSSAsyncHelper<bool>* mkdirHelper = new RSSAsyncHelper<bool>
            (&threadedResult, socket, std::bind(&SFtpChannel::mkdir,
                                        mkdirChannel, directory));
    // Move the socket and SftpChannel to the helper thread.
    socket->moveToThread(mkdirHelper);
    mkdirChannel.moveToThread(mkdirHelper);
    // Connect signal so that we can act appropriately based on result
    // of the command's execution.
    connect(mkdirHelper, SIGNAL(finished()), this, SLOT(announceBooleanResult()));
    // Delete the helper once the event loop of the thread has ended.
    connect(mkdirHelper, SIGNAL(finished()), mkdirHelper, SLOT(deleteLater()));
    mkdirHelper->start();

}

void RemoteServerSession::rmdir(const QString &directory) {

    SFtpChannel rmdirChannel(*socket);
    // Make an AsyncHelper
    RSSAsyncHelper<bool>* rmdirHelper = new RSSAsyncHelper<bool>
            (&threadedResult, socket, std::bind(&SFtpChannel::rmdir,
                                        rmdirChannel, directory));

    // Connect signal so that we can act appropriately based on result
    // of the command's execution.
    connect(rmdirHelper, SIGNAL(finished()), this, SLOT(announceBooleanResult()));
    // Delete the helper once the event loop of the thread has ended.
    connect(rmdirHelper, SIGNAL(finished()), rmdirHelper, SLOT(deleteLater()));
    rmdirHelper->start();
}

void RemoteServerSession::setPurpose(const QString &text) {
    Q_UNUSED(text);

}

void
RemoteServerSession::announceBooleanResult() {
    emit booleanResult(threadedResult);
}

//  MAY NOT NEED THESE TWO METHODS...............
void
RemoteServerSession::promptUserIfMkdirFailed(const bool result) {
//    if (!result) {
//        QMessageBox msgBox;
//        msgBox.setText("There was an issue creating the install directory.<br/>"\
//                       "This likely means that the directory currently exists," \
//                       " which is not good. Please change the install directory.");
//        msgBox.exec();
//    }
}

void
RemoteServerSession::promptUserIfRmdirFailed(const bool result) {
//    if (!result) {
//        QMessageBox msgBox;
//        msgBox.setText("Couldn't remove the directory.");
//        msgBox.exec();
//    }
}



#endif
