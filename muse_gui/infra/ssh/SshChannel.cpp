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
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include <cstdio>

#include <fstream>
#include <array>
#include <algorithm>
#include <iostream>
#include <sstream>

SshChannel::SshChannel(SshSocket& socket) {
    session = socket.getSession();
    channel = NULL;
}

SshChannel::~SshChannel() {
    libssh2_channel_close(channel);
}

bool
SshChannel::copy(const QString &srcData, const QString &destDirectory,
                 const QString &destFileName, const int& mode) {
    // QMessageBox msg;
    //QFileInfo fileInfo(srcDir);

    // Open the file in a read only state.
    //FILE* srcFile = fopen(srcDir.toStdString().c_str(), "r");
    QString remoteFilePath = destDirectory +
            (destDirectory.endsWith("/") ? destFileName : "/" + destFileName);

    int notRead = srcData.length();

    // Send a file via scp. The mode parameter must only have permissions!
    channel = libssh2_scp_send(session, remoteFilePath.toStdString().c_str(), mode,
                               notRead);

    if (channel == nullptr) {
        //msg.setText("Channel problem");
        //msg.exec();
        return false;
    }

    std::array<char, 1024> buffer;
    char* ptr;

    std::stringstream stream{ srcData.toStdString() };

    while (notRead > 0) {
        stream.read(buffer.data(), buffer.size());
        auto readToBuffer = stream.gcount();
        //readToBuffer = fread(buffer, 1, sizeof(buffer), srcFile);

        ptr = buffer.data();

        while (readToBuffer != 0) {
            //write the same data over and over, until error or completion
            auto bytesWritten = libssh2_channel_write(channel, ptr, readToBuffer);

            if (bytesWritten < 0) {
                //msg.setText("byteswritten < 0");
                //msg.exec();
                return false;
            } else {
                ptr += bytesWritten;
                readToBuffer -= bytesWritten;
                notRead -= bytesWritten;
            }
        }
    }

    // Alert the socket that we reached the end of the file.
    libssh2_channel_send_eof(channel);

    // Wait for acknowledgment of the EOF signal
    libssh2_channel_wait_eof(channel);

    return true;
}

bool
SshChannel::copy(const QString &destData, const QString &srcDirectory,
                 const QString &srcFileName) {
    // Creating c-data types for libssh2
    struct stat fileInfo;
    off_t dataReceived = 0;

    // Create a message box to display errors. Will need to be removed
    // if we go with a threaded approach here.
    QMessageBox msgBox;

    // Create the path to the file.
    QString remoteFilePath = srcDirectory +
            (srcDirectory.endsWith("/") ? srcFileName : "/" + srcFileName);

    // Create a file variable and open the file for writing.
    QFile file(destData);
    if (!file.open(QIODevice::WriteOnly)) {
        msgBox.setText("Couldn't create or open the file on the local system.");
        msgBox.exec();

        // We can't put the file here, bail out.
        return false;
    }

    // Tell the remote server we wish to receive a file from it.
    channel = libssh2_scp_recv(session, remoteFilePath.toStdString().c_str(), &fileInfo);

    // Prepare the datastream.
    QDataStream stream(&file);

    if (!channel) {
        msgBox.setText("Channel issue");
        msgBox.exec();

        // Problem, bail out
        return false;
    }

    while(dataReceived < fileInfo.st_size) {
        // A buffer to hold the data received
        std::array<char, 1024> mem;
        int rc;

        // the amount to receive
        int amount = mem.size();

        if ((fileInfo.st_size - dataReceived) < amount) {
            // if there is more space in the buffer than there
            // is left to read, set that number to amount
            amount = fileInfo.st_size - dataReceived;
        }

        // Read "amount" data from the channel
        rc = libssh2_channel_read(channel, mem.data(), amount);

        // If we received data, write it to the file and update
        // the dataReceived count.
        if (rc > 0) {
            stream.writeBytes(mem.data(), rc);
            dataReceived += rc;
        }
    }

    // Close the file.
    file.close();
    return true;
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

        std::array<char, 0x4000> stdBuffer;
        std::fill(std::begin(stdBuffer), std::end(stdBuffer), ' ');

        libssh2_channel_read_ex(channel, 0, stdBuffer.data(), stdBuffer.size());

        stdoutput = stdBuffer.data();

        std::array<char, 0x4000> errBuffer;
        std::fill(std::begin(errBuffer), std::end(errBuffer), ' ');

        libssh2_channel_read_stderr(channel, errBuffer.data(), errBuffer.size());

        stderrmsgs = errBuffer.data();

        return returnCode;
    }

    // Arbitrary value for fail code.
    return -1000;
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

        std::array<char, 0x4000> stdBuffer;
        std::fill(std::begin(stdBuffer), std::end(stdBuffer), ' ');

        libssh2_channel_read_ex(channel, 0, stdBuffer.data(), stdBuffer.size());

        // Add the text to the QTextEdit
        output.append(stdBuffer.data());

        std::array<char, 0x4000> errBuffer;
        std::fill(std::begin(errBuffer), std::end(errBuffer), ' ');

        libssh2_channel_read_stderr(channel, errBuffer.data(), errBuffer.size());

        // Add the error text to the QTextEdit
        output.append(errBuffer.data());

        return returnCode;
    }

    // Arbitrary value for fail code.
    return -1000;
}

#endif
