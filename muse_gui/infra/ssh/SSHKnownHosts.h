#ifndef SSH_KNOWN_HOSTS_H
#define SSH_KNOWN_HOSTS_H

#include <QObject>
#include <libssh2.h>

#include "SshException.h"

// Forward declaration to avoid circular dependency
class SshSocket;
class SshException;

class SSHKnownHosts : public QObject {
    Q_OBJECT

public:
    SSHKnownHosts(const QString& knownHostsFile = "known_hosts",
                  bool handleSignals = true,
                  Qt::ConnectionType conType = Qt::AutoConnection);
    ~SSHKnownHosts();

    bool isKnownHost(SshSocket& ssh, LIBSSH2_SESSION *sshSession)
    throw (const SshException &);

    static QString format(const QString& toFormat, const QStringList& values);

    void close();

public slots:
    void checkConnectionToUnknownHost(const bool newHost, SshSocket& ssh,
                                      LIBSSH2_SESSION *sshSession,
                                      libssh2_knownhost* hostInfo,
                                      bool& cancel);

signals:
    void sshUnknownHostDetected(const bool newHost,
                                SshSocket& ssh, LIBSSH2_SESSION *sshSession,
                                libssh2_knownhost* hostInfo,
                                bool& cancel);

protected:
    bool loadKnownHosts(SshSocket &ssh, LIBSSH2_SESSION* sshSession)
    throw (const SshException &);
    int  checkHost(SshSocket& ssh, LIBSSH2_SESSION* sshSession,
                   libssh2_knownhost **hostInfo);
    bool checkUpdateKnownHosts(SshSocket& ssh, LIBSSH2_SESSION* sshSession,
                               const QString& msgFormat,
                               libssh2_knownhost *hostInfo);
    bool addKnownHost(SshSocket& ssh, LIBSSH2_SESSION* sshSession,
                      libssh2_knownhost *currEntry = NULL);
    QString toHex(const char *data, const int len = 20) const;

private:
    const QString knownHostsFileName;
    LIBSSH2_KNOWNHOSTS *knownHostsList;

    static const QString KnownHostLoadErrorMessage;
    static const QString UnableToSaveKnownHostsMessage;
    static const QString NewHostMessage;
    static const QString HostInfoChangedMessage;
    static const QString ConnectDetailsMessage;
    static const QString ConnectQuestionMessage;
};

#endif // SSH_KNOWN_HOSTS_H
