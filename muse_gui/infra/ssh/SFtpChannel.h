#ifndef SFTP_CHANNEL_H
#define SFTP_CHANNEL_H

#include "SFtpDir.h"

// Forward declaration to avoid circular dependency
class SshSocket;
class SFtpChannel;

class SFtpChannel : public QObject {
    Q_OBJECT
public:
    SFtpChannel(SshSocket& ssh) throw (const SshException &);
    ~SFtpChannel();
    QString getPwd() throw (const SshException &);
    QString getErrorMessage() const throw ();
    SshSocket& getSocket() { return ssh; }
    LIBSSH2_SFTP* getChannel() const { return sftpChannel; }
    SFtpDir getDir(const QString& dir) throw (const SshException &);

protected:

private:
    SshSocket& ssh;
    LIBSSH2_SFTP* sftpChannel;
    int error;
};

#endif // SFTP_H
