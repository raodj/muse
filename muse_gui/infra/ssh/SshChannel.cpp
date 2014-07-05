#ifndef SSH_CHANNEL_CPP
#define SSH_CHANNEL_CPP

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

#include "SshChannel.h"

SshChannel::SshChannel(SshSocket& socket) {
    session = socket.getSession();
    channel = NULL;
}

SshChannel::~SshChannel() {
    libssh2_channel_close(channel);
}

void
SshChannel::copy(std::istream &srcData, const QString &destDirectory,
                 const QString &destFileName, const QString &mode) {

    Q_UNUSED(srcData);
    Q_UNUSED(destDirectory);
    Q_UNUSED(destFileName);
    Q_UNUSED(mode);
}

void
SshChannel::copy(std::ostream &destData, const QString &srcDirectory,
                 const QString &srcFileName) {
    Q_UNUSED(destData);
    Q_UNUSED(srcDirectory);
    Q_UNUSED(srcFileName);

}

int
SshChannel::exec(const QString &command, QString &stdoutput,
                 QString &stderrmsgs) {
    // If the channel hasn't been opened...
    if (channel == NULL) {
        // Create the communication channel
        channel = libssh2_channel_open_session(session);
    }
    // This can't be an else because the attempt to open the channel
    // could fail.
    if (channel != NULL) {
        int returnCode = -100;
        // Read the streams if the command was executed successfully.
        returnCode = libssh2_channel_exec(
                    channel, command.toStdString().c_str());
        char stdBuffer[0x4000];
        libssh2_channel_read_ex(channel, 0, stdBuffer, sizeof(stdBuffer));
        stdoutput = stdBuffer;
        char errBuffer[0x4000];
        libssh2_channel_read_stderr(channel, errBuffer, sizeof(errBuffer));
        stderrmsgs = errBuffer;
        return returnCode;
    }
}

int
SshChannel::exec(const QString &command, QTextEdit &output) {
    // If the channel hasn't been opened...
    if (channel == NULL) {
        // Create the communication channel
        channel = libssh2_channel_open_session(session);
    }
    // This can't be an else because the attempt to open the channel
    // could fail.
    if (channel != NULL) {
        int returnCode = -100;
        // Read the streams if the command was executed successfully.
        returnCode = libssh2_channel_exec(
                    channel, command.toStdString().c_str());
        char stdBuffer[0x4000];
        libssh2_channel_read_ex(channel, 0, stdBuffer, sizeof(stdBuffer));
        // Add the text to the QTextEdit
        output.append(stdBuffer);
        char errBuffer[0x4000];
        libssh2_channel_read_stderr(channel, errBuffer, sizeof(errBuffer));
        // Add the error text to the QTextEdit
        output.append(errBuffer);
        return returnCode;
    }
}

#endif
