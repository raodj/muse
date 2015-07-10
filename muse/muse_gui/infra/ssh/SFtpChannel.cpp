#ifndef SFTP_CHANNEL_CPP
#define SFTP_CHANNEL_CPP

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

#include "SFtpChannel.h"
#include "SshSocket.h"

#define MAX_PATH_LEN 1024
#define SUCCESS_CODE 0

SFtpChannel::SFtpChannel(SshSocket& ssh) : ssh(ssh) {
    sftpChannel = libssh2_sftp_init(ssh.getSession());
    if (sftpChannel == NULL) {
        throw SSH_EXP(ssh, getErrorMessage(), ssh.getErrorCode(), ssh.error());
    }
}

SFtpChannel::~SFtpChannel() {
    if (sftpChannel != NULL) {
        libssh2_sftp_shutdown(sftpChannel);
        sftpChannel = NULL;
    }
}

SFtpChannel::SFtpChannel(const SFtpChannel &channel) : ssh(channel.getSocket()) {
    sftpChannel = libssh2_sftp_init(ssh.getSession());
    if (sftpChannel == NULL) {
        throw SSH_EXP(ssh, getErrorMessage(), ssh.getErrorCode(), ssh.error());
    }
}

QString
SFtpChannel::getErrorMessage() const {
    QString errMsg(ssh.getErrorMessage());
    if (error == LIBSSH2_ERROR_SFTP_PROTOCOL) {
        errMsg += "[SFftp error code: ";
        errMsg += error;
        errMsg += "]";
    }
    return errMsg;
}

QString
SFtpChannel::getPwd() {
    char tmpPath[MAX_PATH_LEN];
    if ((error = libssh2_sftp_realpath(sftpChannel, ".",
                                       tmpPath, MAX_PATH_LEN)) <= 0) {
        throw SSH_EXP(ssh, getErrorMessage(), ssh.getErrorCode(), ssh.error());
    }
    // Successfully determined current working directory
    return QString(tmpPath);
}

SFtpDir
SFtpChannel::getDir(const QString &dir) {
    return SFtpDir(*this, dir);
}

bool
SFtpChannel::mkdir(const QString& dir) {
        int returnCode = libssh2_sftp_mkdir(sftpChannel, dir.toStdString().c_str(), 0700);
        // If the directory couldn't be created, we have a problem,
        // so alert the user and bail out.
        if (returnCode != SUCCESS_CODE) {
            emit alertUser("There was an issue creating the specified directory.<br/>"\
                           "This likely means that the directory currently exists," \
                           " which is not good. Please change the name of the directory.");
            return false;
        }
        // Success! We made the directory!
        return true;
}

bool
SFtpChannel::mkdirs(const QStringList &dirs) {
    for (const auto& dir : dirs) {
        if (!mkdir(dir)) {
            return false;
        }
    }

    return true;
}

bool
SFtpChannel::rmdir(const QString &dir) {
        int returnCode = libssh2_sftp_rmdir(sftpChannel, dir.toStdString().c_str());
        // If the directory couldn't be removed, we have a problem,
        // so alert the user and bail out.
        if (returnCode != SUCCESS_CODE) {
            emit alertUser("There was an issue deleting the specified directory.<br/>"\
                           "Please verify that the directory exists. If it does," \
                           " verify your connection to the server.");
            return false;
        }
        // Success! We removed the directory!
        return true;
}

bool
SFtpChannel::dirExists(const QString& dir) {
    // libssh2_sftp_opendir will return a LIBSSH2_SFTP_HANDLE* if the directory
    // was opened, or null if it was not
    return libssh2_sftp_opendir(sftpChannel, dir.toStdString().c_str()) != nullptr;
}

bool
SFtpChannel::dirsExist(QStringList dirs) {
    for (const auto& dir : dirs) {
        if (!dirExists(dir)) {
            return false;
        }
    }

    return true;
}

#endif
