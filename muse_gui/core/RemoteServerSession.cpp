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
#include "RemoteServerSession.h"
#include "MUSEGUIApplication.h"
#include <QObject>


RemoteServerSession::RemoteServerSession(Server &server, QWidget *parent, QString purpose) :
    ServerSession(server, parent), purpose(purpose), connectionThread(server){

}

void RemoteServerSession::connectToServer() {
    connectionThread.start();
}

void RemoteServerSession::disconnectFromServer() {

}

void RemoteServerSession::getPassword() {

}

int RemoteServerSession::exec(const QString &command, QString &stdoutput,
                              QString &stderrmsgs) {
    Q_UNUSED(command);
    Q_UNUSED(stdoutput);
    Q_UNUSED(stderrmsgs);

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

void RemoteServerSession::mkdir(const QString &directory) {
    Q_UNUSED(directory);

}

void RemoteServerSession::rmdir(const QString &directory) {
    Q_UNUSED(directory);

}

void RemoteServerSession::setPurpose(const QString &text) {
    Q_UNUSED(text);

}

#endif
