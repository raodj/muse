#include "SFtpChannel.h"
#include "SshSocket.h"

#define MAX_PATH_LEN 1024

SFtpChannel::SFtpChannel(SshSocket& ssh) throw (const SshException &) :
    ssh(ssh) {
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

QString
SFtpChannel::getErrorMessage() const throw () {
    QString errMsg(ssh.getErrorMessage());
    if (error == LIBSSH2_ERROR_SFTP_PROTOCOL) {
        errMsg += "[SFftp error code: ";
        errMsg += error;
        errMsg += "]";
    }
    return errMsg;
}

QString
SFtpChannel::getPwd() throw (const SshException &) {
    char tmpPath[MAX_PATH_LEN];
    if ((error = libssh2_sftp_realpath(sftpChannel, ".",
                                       tmpPath, MAX_PATH_LEN)) <= 0) {
        throw SSH_EXP(ssh, getErrorMessage(), ssh.getErrorCode(), ssh.error());
    }
    // Successfully determined current working directory
    return QString(tmpPath);
}

SFtpDir
SFtpChannel::getDir(const QString &dir) throw (const SshException &) {
    return SFtpDir(*this, dir);
}
