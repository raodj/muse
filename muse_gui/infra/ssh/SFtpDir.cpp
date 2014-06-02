#include "SFtpChannel.h"
#include "SFtpDir.h"
#include "SshSocket.h"

SFtpDir::SFtpDir(SFtpChannel &sftp, const QString &dir) throw (const SshException &)
    : sftp(sftp), dir(dir), goodFlag(true) {
    if ((sftpDir = libssh2_sftp_opendir(sftp.getChannel(),
                                        dir.toStdString().c_str())) == NULL) {
        goodFlag = false;
        QString msg = "SFTP was unable to open directory: " + dir;
        throw SSH_EXP(sftp.getSocket(), msg, sftp.getSocket().getErrorCode(),
                      sftp.getSocket().error());
    }
}

SFtpDir::~SFtpDir() throw () {
    if (sftpDir != NULL) {
        libssh2_sftp_closedir(sftpDir);
        sftpDir  = NULL;
        goodFlag = false; // no good anymore.
    }
}

void
SFtpDir::getInfo(long &size, long &timestamp, int &flags)
throw (const SshException &) {
    // First check to ensure that the current status of dir is good.
    if (!good() || (sftpDir == NULL)) {
        QString msg = "SFTP connection is not good to stat: " + dir;
        throw SSH_EXP2(sftp.getSocket(), msg);
    }
    // In order to get info, the dir must be opened as a file for reading.
    // This is performed as a separate sftp handle.
    LIBSSH2_SFTP_HANDLE* const sftpFile =
            libssh2_sftp_open(sftp.getChannel(), dir.toStdString().c_str(),
                              LIBSSH2_FXF_READ, 0);
    if (sftpFile == NULL) {
        goodFlag = false;
        QString msg = "SFTP was unable to open path: " + dir +
                "\nError: " + sftp.getErrorMessage();
        throw SSH_EXP(sftp.getSocket(), msg, sftp.getSocket().getErrorCode(),
                      sftp.getSocket().error());
    }
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    if ((sftpFile == NULL) || (libssh2_sftp_fstat(sftpFile, &attrs) != 0)) {
        goodFlag = false;
        QString msg = "SFTP was unable to stat directory: " + dir +
                "\nError: " + sftp.getErrorMessage();
        throw SSH_EXP(sftp.getSocket(), msg, sftp.getSocket().getErrorCode(),
                      sftp.getSocket().error());
    }
    // Close the sftp connection as it is not needed
    libssh2_sftp_close_handle(sftpFile);
    // Update parameters with necessary information.
    size      = (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE) ? attrs.filesize : -1;
    timestamp = (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME) ? attrs.mtime : -1;
    flags     = (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) ?
                attrs.permissions : -1;
}

bool
SFtpDir::getEntry(QString& file, long &size, long &timestamp, int &flags)
throw (const SshException &) {
    if (!good() || (sftpDir == NULL)) {
        // The class is not in good state. Nothing further to do.
        return false;
    }
    // Try and read the next entry in the directory opened in the constructor.
    char name[1024];
    LIBSSH2_SFTP_ATTRIBUTES attrs;
    if (libssh2_sftp_readdir(sftpDir, name, sizeof(name), &attrs) > 0) {
        // Read information successfully. Update parameters with values
        file      = name;
        size      = (attrs.flags & LIBSSH2_SFTP_ATTR_SIZE) ? attrs.filesize : -1;
        timestamp = (attrs.flags & LIBSSH2_SFTP_ATTR_ACMODTIME) ? attrs.mtime : -1;
        flags     = (attrs.flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) ?
                    attrs.permissions : -1;
        return true;
    }
    // Can't read any more entries and data in parameters is not valid.
    return (goodFlag = false);
}

