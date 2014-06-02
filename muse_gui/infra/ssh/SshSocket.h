#ifndef SSH_H
#define SSH_H

#include <QTcpSocket>
#include <QWidget>
#include <libssh2.h>

#include "SSHKnownHosts.h"
#include "SshException.h"
#include "SFtpChannel.h"

// Forward declarations to keep compile fast
class QProgressDialog;

/**
 * @brief The Secure Shell (SSH) class that streamlines interactions
 * with remote hosts over a secure connection.
 *
 *
 */
class SshSocket : public QTcpSocket {
    Q_OBJECT
public:
    SshSocket(const QString& reason = tr("MUSE GUI Operations"),
              QWidget* parent = NULL,
              const QString& knownHostsFile = ".known_hosts",
              bool handleSignals = true,
              Qt::ConnectionType connType = Qt::AutoConnection) throw ();
    ~SshSocket() throw ();

    bool connectToHost(const QString &hostName,
                       QProgressDialog *progDiag = NULL, quint16 port = 22,
                       OpenMode mode = ReadWrite) throw (const SshException &);

    QWidget* getOwner() const throw () { return owner; }
    const QString& getReason() const throw () { return reason; }
    QString getErrorMessage(const QString& prefix = "") const throw ();
    int getErrorCode() const { return libssh2_session_last_errno(sshSession); }

    static const QString Title;

    LIBSSH2_SESSION* getSession() { return sshSession; }

public slots:
    void getCredentials(SshSocket& ssh, const libssh2_knownhost* hostInfo,
                        QString& userID, QString& password, bool& cancel);

signals:
    void sshHandshakeCompleted(SshSocket& ssh);
    void needCredentials(SshSocket& ssh, const libssh2_knownhost* hostInfo,
                         QString& userID, QString& password, bool& cancel);

protected:
    bool authenticate(LIBSSH2_SESSION *sshSession) throw (const SshException&);

private:
    LIBSSH2_SESSION *sshSession;
    SSHKnownHosts knownHosts;
    QWidget* const owner;
    const QString reason;
    static const QString ConnectErrorMessage;
    static const QString GenericErrorMessage;
};

#endif // SSH_H
