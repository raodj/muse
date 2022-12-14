#ifndef SSH_H
#define SSH_H

#include <QTcpSocket>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
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
              QWidget* parent = nullptr,
              const QString& knownHostsFile = "known_hosts",
              bool handleSignals = true, bool runInSeparateThread = false,
              Qt::ConnectionType connType = Qt::AutoConnection) noexcept;

    ~SshSocket() noexcept;

    bool connectToHost(const QString &hostName,
                       QProgressDialog *progDiag = nullptr, quint16 port = 22,
                       OpenMode mode = ReadWrite);

    QWidget* getOwner() const noexcept { return owner; }
    const QString& getReason() const noexcept { return reason; }
    QString getErrorMessage(const QString& prefix = "") const noexcept;
    int getErrorCode() const { return libssh2_session_last_errno(sshSession); }

    static const QString Title;

    LIBSSH2_SESSION* getSession() { return sshSession; }

    /**
     * @brief getKnownHosts Made for the purpose of handling threaded
     * uses of this class, allows the user to connect signals from the
     * known hosts to their own implementations as needed.
     * @return The knownHosts.
     */
    SSHKnownHosts* getKnownHosts() { return &knownHosts; }

    /**
     * @brief changeToThread Moves this SshSocket and it's member variables
     * to QThread thread.
     * @param thread The thread to move this SshSocket to.
     */
    void changeToThread(QThread* thread);

public slots:
    void getCredentials(SshSocket& ssh, const libssh2_knownhost* hostInfo,
                        QString& userID, QString& password, bool& cancel);


signals:
    void sshHandshakeCompleted(SshSocket& ssh);
    void needCredentials(SshSocket& ssh, const libssh2_knownhost* hostInfo,
                         QString& userID, QString& password, bool& cancel);
    /**
     * @brief needCredentials An overloaded method of needCredentials()
     * that is to be used when connectToHost() is running a thread other
     * than the Qt main thread.<br/>
     *
     * <b>NOTE</b> This signal must be handled in an appropriate way
     * by the implementer. At this time, SshSocket is expecting the
     * implementer will perform the necessary operations to supply
     * this instance of SshSocket with the credentials needed to
     * connect to the remote host.
     *
     * @param username A pointer to the username credential.
     * @param password A pointer to the password credential.
     */
    void needCredentials(QString* username, QString* password);


protected:
    bool authenticate(LIBSSH2_SESSION *sshSession);

private:
    LIBSSH2_SESSION *sshSession;
    SSHKnownHosts knownHosts;
    QWidget* const owner;
    const QString reason;
    static const QString ConnectErrorMessage;
    static const QString GenericErrorMessage;
    bool runInSeparateThread;
};

#endif // SSH_H
