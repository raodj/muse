#include "SshSocket.h"
#include "SSHKnownHosts.h"
#include "LoginCredentialsDialog.h"

#include <QProgressDialog>
#include <QHostAddress>
#include <QMessageBox>

// A commonly used title for GUI display purposes
const QString SshSocket::Title(tr("SSH Library"));

// Error message to be formatted and displayed when connection fails
const QString SshSocket::ConnectErrorMessage =
        "Unable to connect to host %1 at port %2 <br/>" \
        "Here are a few troubleshooting tips: <ul>" \
        "<li>Ensure you have network connectivity</li>" \
        "<li>Verify the host name and port are valid</li>" \
        "<li>Check if the remote host accepts connections on the port.</li>"\
        "</ul>";

const QString SshSocket::GenericErrorMessage =
        "An error occurred when creating or using an SSH socket. " \
        "The message above provides some information about the "   \
        "source of the error. Please try to resolve the source "   \
        "of the error and try the operation again.";

SshSocket::SshSocket(const QString& reason, QWidget *parent,
                     const QString& knownHostsFile, bool handleSignals,
                     Qt::ConnectionType connType) throw () :
    QTcpSocket(NULL), knownHosts(knownHostsFile, handleSignals, connType),
    owner(parent), reason(reason) {
    sshSession = NULL;
    if (handleSignals) {
        connect(this, SIGNAL(needCredentials(SshSocket&,
                      const libssh2_knownhost*, QString&, QString&, bool&)),
                this, SLOT(getCredentials(SshSocket&,
                      const libssh2_knownhost*, QString&, QString&, bool&)),
                connType);
    }
}
SshSocket::~SshSocket() throw () {
    if (sshSession != NULL) {
        libssh2_session_disconnect(sshSession, reason.toStdString().c_str());
        libssh2_session_free(sshSession);
        qDebug() << "SSH socket deleted.";
        sshSession = NULL;
    }
}

QString
SshSocket::getErrorMessage(const QString &prefix) const throw() {
    QString errorMsg(prefix);
    // Report errors from both the socket layer
    if (error() != UnknownSocketError) {
        errorMsg.append(errorString());
    }
    // Add errors from the SSH2 library layer (if possible)
    if (sshSession != NULL) {
        char *sshErrMsg;
        int  errMsgLen = 0;
        libssh2_session_last_error(sshSession, &sshErrMsg, &errMsgLen, 0);
        errorMsg.append("SSH2 Error: ");
        errorMsg.append(sshErrMsg);
    }
    // Send the
    return errorMsg;
}

#define SET_PROGRESS(value) if (progDiag != NULL) { \
    qDebug() << "Progress value = " << value; \
    progDiag->setValue(value); \
}

bool
SshSocket::connectToHost(const QString &hostName,
                         QProgressDialog *progDiag, quint16 port,
                         OpenMode mode) throw (const SshException &) {
    SET_PROGRESS(1);
    // Initialize libssh2 library. Maybe this needs to be done just once
    if (libssh2_init(0) != 0) {
        // Error initializing libssh2! Cannot proceed further.
        throw SSH_EXP2(*this, "Error initializing SSH library");
    }
    SET_PROGRESS(2);
    // First create a socket connected to the remote host on the given port.
    QTcpSocket::connectToHost(hostName, port, mode);
    if (!waitForConnected(10000) || (state() != ConnectedState)) {
        QString portNumStr = QString::number(port);
        throw SSH_EXP2(*this, ConnectErrorMessage.arg(hostName, portNumStr));
    }
    SET_PROGRESS(3);
    if ((sshSession = libssh2_session_init()) == NULL) {
        // ssh2 session initialization failed.
        QString errMsg = getErrorMessage("Error initalizing SSH session: ");
        throw SSH_EXP(*this, errMsg, getErrorCode(), UnknownSocketError);
    }
    SET_PROGRESS(4);
    // It is time to startup the session, verify remote host, & authenticate
    Q_ASSERT( sshSession != NULL );
    libssh2_session_flag(sshSession, LIBSSH2_FLAG_COMPRESS, 1);
    if (libssh2_session_handshake(sshSession, socketDescriptor())) {
        // Initial handshake over SSH was *not* successful.
        QString errMsg = getErrorMessage("Initial SSH handshake failed: ");
        throw SSH_EXP(*this,  errMsg, getErrorCode(), UnknownSocketError);
    }
    SET_PROGRESS(5);
    // Time to verify host signature before proceeding with authentication.
    if (!knownHosts.isKnownHost(*this, sshSession)) {
        // Host signature verification failed. Bail out
        knownHosts.close(); // Clear out unnecessary values
        // emit error(*this, "Known host verification process failed.");
        return false;
    }
    knownHosts.close(); // Clear out unnecessary values
    SET_PROGRESS(6);
    // Now use different forms of authentication and try to login and return
    // result of this process.
    return authenticate(sshSession);
}

bool
SshSocket::authenticate(LIBSSH2_SESSION *sshSession)
throw (const SshException &){
    // Maybe we should check for different forms of authentication?
    // Now get login credentials from the user.
    QString username = "dmadhava", password;
    bool cancel = false;
    emit needCredentials(*this, NULL, username, password, cancel);
    qDebug() << "username: " << username << ", password: *"
             << "cancel: "   << cancel;
    if (cancel) {
        // The user cancelled the operation!
        return false;
    }
    // Try to login over ssh and report issues if any.
    if (libssh2_userauth_password(sshSession, username.toStdString().c_str(),
                                  password.toStdString().c_str()) != 0) {
        // The login was unsuccessful. Should we try this again?
        QString errMsg = getErrorMessage("Login failed: ");
        throw SSH_EXP(*this, errMsg, getErrorCode(), UnknownSocketError);
        return false;
    }
    // Authentication successful
    qDebug() << "Authentication successful!";
    return true;
}

void
SshSocket::getCredentials(SshSocket &ssh, const libssh2_knownhost *hostInfo,
                          QString &userID, QString &password, bool &cancel) {
    Q_UNUSED(ssh);
    Q_UNUSED(hostInfo);
    LoginCredentialsDialog lcd(owner);
    lcd.setUsername(userID);
    // Load other IDs that the user has used in the past here. For now
    // let's add some dummy values for testing.
//    QStringList otherIDs;
//    otherIDs << "superman" << "batman" << "007";
//    lcd.setUsernamesList(otherIDs);
    lcd.setPassword(password);
    if (lcd.exec() != QDialog::Accepted) {
        // User canceled operation.
        cancel = true;
    } else {
        // Update the values with the current values entered by the user.
        userID   = lcd.getUserName();
        password = lcd.getPassword();
        cancel   = false;
    }
}

