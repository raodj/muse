#include "DirectoryASyncHelper.h"
#include "libssh2.h"
#include "libssh2_sftp.h"

#define SUCCESS_CODE 0

DirectoryASyncHelper::DirectoryASyncHelper(const QString methodToCall,
                                           const QString& directory,
                                           SshSocket* socket) :
    method(methodToCall), directory(directory){
    this->socket = socket;
    success = false;
}

void
DirectoryASyncHelper::run() {
    if (method == "mkdir") {
        success = mkdir();
    }
    else {
        success = rmdir();
    }
    emit result(success);
}

bool DirectoryASyncHelper::mkdir() {
    // Create an sftp variable
    LIBSSH2_SFTP* sftpSession = NULL;
    // Attempt to set it up
    sftpSession = libssh2_sftp_init(socket->getSession());
    // Check for success
    if (sftpSession != NULL) {
        int returnCode = libssh2_sftp_mkdir(sftpSession, directory.toStdString().c_str(), 0700);
        //QMessageBox msgBox;
        if (libssh2_sftp_shutdown(sftpSession) != SUCCESS_CODE ||
                returnCode != SUCCESS_CODE) {
            /*msgBox.setText("There was an issue creating the install directory.<br/>"\
                           "This likely means that the directory currently exists," \
                           " which is not good. Please change the install directory."); */
            // msgBox.exec();
            return false;
        }
        return true;
    }
    return false;
}

bool
DirectoryASyncHelper::rmdir() {
    // Create an sftp variable
    LIBSSH2_SFTP* sftpSession = NULL;
    // Attempt to set it up
    sftpSession = libssh2_sftp_init(socket->getSession());
    // Check for success
    if (sftpSession != NULL) {
        int returnCode = libssh2_sftp_rmdir(sftpSession, directory.toStdString().c_str());
        //QMessageBox msgBox;
        if (libssh2_sftp_shutdown(sftpSession) != SUCCESS_CODE ||
                returnCode != SUCCESS_CODE) {
            /*msgBox.setText("There was an issue creating the install directory.<br/>"\
                           "This likely means that the directory currently exists," \
                           " which is not good. Please change the install directory."); */
            // msgBox.exec();
            return false;
        }
        return true;
    }
    return false;
}
