#ifndef SFTP_DIR_H
#define SFTP_DIR_H

#include <libssh2_sftp.h>
#include "SshException.h"

// Forward declaration to avoid recursive includes
class SFtpChannel;

class SFtpDir {
public:
    SFtpDir(SFtpChannel& sftp, const QString& dir) throw (const SshException &);
    ~SFtpDir() throw ();
    void getInfo(long& size, long& timestamp, int& flags)
    throw (const SshException &);
    bool getEntry(QString& file, long& size, long& timestamp, int& flags)
    throw (const SshException &);
    bool good() const { return goodFlag; }
    QString getPath() const { return dir; }

private:
    SFtpChannel& sftp;
    QString dir;
    bool goodFlag;
    LIBSSH2_SFTP_HANDLE *sftpDir;
};

#endif // SFTP_DIR_H
