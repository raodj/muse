#ifndef SSH_KNOWN_HOSTS_CPP
#define SSH_KNOWN_HOSTS_CPP

#include "SSHKnownHosts.h"
#include "SshSocket.h"
#include <QHostAddress>
#include <QMessageBox>

const QString SSHKnownHosts::KnownHostLoadErrorMessage =
        "Unable to load list of known hosts from the known hosts "  \
        "file:<br/> <tt>%1</tt><br/><br/>"                          \
        "<i>This is not a common occurrence and may be indicative " \
        "possible hacking attempt. Check the above file manually "  \
        "and remove all entries in the file. Once all entries have "\
        "been cleared try to repeat the desired operation(s).";

const QString SSHKnownHosts::UnableToSaveKnownHostsMessage =
        "Unable to save list of known hosts from the known hosts "  \
        "file:<br/> <tt>%1</tt><br/><br/>"                          \
        "<i>This is not a common occurrence and may be indicative " \
        "potential errors. Check the above file and disk manually "  \
        "to ensure they are in good state. Once you have verfied "\
        "the system try to repeat the desired operation(s).";

const QString SSHKnownHosts::NewHostMessage =
        "It appears that you are connecting to %1 (port: %3, " \
        "Address: %2) for the first time (because an entry "   \
        "for this host was not found in the known hosts file).";

const QString SSHKnownHosts::ConnectDetailsMessage =
        "Information about the connection being established:\n"    \
        "    Connect reason: %7\n"                                 \
        "    Host information: %1 (Address: %2)\n"                 \
        "    New fingerprint: %5\n"                                \
        "    Old fingerprint: %6\n"                                \
        "    Known hosts file: %4";


const QString SSHKnownHosts::HostInfoChangedMessage =
        "It appears that the fingerprint information of the host "  \
        "you are trying to connecting to [%1 (port: %2)] has     "  \
        "changed! The current fingerprint of the host is not the "  \
        "same as the value in the known hosts file. "               \
        "This is indicative of some form of attack or an  "  \
        "hacking attempt.  However, it is also possible that it  "  \
        "a legitimate change in the host's fignerprint.";

const QString SSHKnownHosts::ConnectQuestionMessage =
        "Do you trust the remote host and would you like to "    \
        "update its information (in the known hosts file) and "  \
        "proceed connecting this with this host?";

SSHKnownHosts::SSHKnownHosts(const QString &knownHostsFile,
                             bool handleSignals, Qt::ConnectionType conType) :
    knownHostsFileName(knownHostsFile) {
    knownHostsList = NULL;
    if (handleSignals) {
        // Connect signal about new or changed host signal to the default
        // slot that checks with the user about connecting to the host.
        connect(this, SIGNAL(sshUnknownHostDetected(bool, SshSocket&,
                             LIBSSH2_SESSION*, libssh2_knownhost*, bool&)),
                this, SLOT(checkConnectionToUnknownHost(bool, SshSocket&,
                         LIBSSH2_SESSION*, libssh2_knownhost*, bool&)),
                conType);
    }
}

SSHKnownHosts::~SSHKnownHosts() {
    close();
}

void
SSHKnownHosts::close() {
    if (knownHostsList != NULL) {
        libssh2_knownhost_free(knownHostsList);
        knownHostsList = NULL;
    }
}

bool
SSHKnownHosts::loadKnownHosts(SshSocket& ssh, LIBSSH2_SESSION *sshSession)
throw (const SshException &) {
    Q_ASSERT( sshSession != NULL );
    // If we have already loaded the list then there is nothing further to do
    if (knownHostsList != NULL) {
        return true;
    }
    // Iniitalize the known hosts list
    knownHostsList = libssh2_knownhost_init(sshSession);
    const char* hostsFileName = knownHostsFileName.toStdString().c_str();
    if ((knownHostsList == NULL) ||
        (libssh2_knownhost_readfile(knownHostsList, hostsFileName,
                                    LIBSSH2_KNOWNHOST_FILE_OPENSSH) < 0)) {
        // Error initializing or loading known hosts. Bail out.
        throw SSH_EXP2(ssh, KnownHostLoadErrorMessage.arg(knownHostsFileName));
        return false;
    }
    // Everything went fine.
    return true;
}

int
SSHKnownHosts::checkHost(SshSocket &ssh, LIBSSH2_SESSION *sshSession,
                         libssh2_knownhost **hostInfo) {

    // Get the SSH fingerprint for the remote host for checking.
    size_t hostKeyLen   = 0;
    int hostKeyType     = -1;
    const char *hostKey = libssh2_session_hostkey(sshSession, &hostKeyLen,
                                                  &hostKeyType);
    // Get the host name as C-string to pass to libSSH2
    const char *hostName = ssh.peerName().toStdString().c_str();
    // Check if the host is in the known-list of hosts.
    const int result =
            libssh2_knownhost_checkp(knownHostsList, hostName, ssh.peerPort(),
                                     hostKey, hostKeyLen,
                                     LIBSSH2_KNOWNHOST_TYPE_PLAIN |
                                     LIBSSH2_KNOWNHOST_KEYENC_RAW, hostInfo);
    // Return the result of the check
    return result;
}

bool
SSHKnownHosts::isKnownHost(SshSocket& ssh, LIBSSH2_SESSION *sshSession)
throw (const SshException &) {
    Q_ASSERT( sshSession != NULL );
    // Iniitalize the known hosts list
    if (!loadKnownHosts(ssh, sshSession)) {
        // Error initializing or loading known hosts
        QString msg = KnownHostLoadErrorMessage.arg(knownHostsFileName);
        throw SSH_EXP2(ssh, msg);
    }
    // Check if host is in the "known hosts" list and if not take some actions
    libssh2_knownhost *hostInfo = NULL;
    const int status = checkHost(ssh, sshSession, &hostInfo);
    bool cancel      = true; // updated in the switch below
    switch (status) {
    case LIBSSH2_KNOWNHOST_CHECK_FAILURE:
        throw SSH_EXP2(ssh, KnownHostLoadErrorMessage.arg(knownHostsFileName));
        break;
    case LIBSSH2_KNOWNHOST_CHECK_NOTFOUND:
        emit sshUnknownHostDetected(true, ssh, sshSession, hostInfo, cancel);
        break;
    case LIBSSH2_KNOWNHOST_CHECK_MISMATCH:
        emit sshUnknownHostDetected(false, ssh, sshSession, hostInfo, cancel);
        break;
    case LIBSSH2_KNOWNHOST_CHECK_MATCH:
        cancel = false;
    }
    // Return result of various conditions from the switch statement above
    return !cancel;
}

QString
SSHKnownHosts::toHex(const char *data, const int len) const {
    // Convert given data to a hexadecimal string.
    QByteArray temp(data, len);
    return QString(temp.toHex());
}

QString
SSHKnownHosts::format(const QString &toFormat, const QStringList &values) {
    QString retVal = toFormat;
    for(int i = 0; (i < values.size()); i++) {
        QString metastr = "%";
        metastr.append(QString::number(i + 1));
        retVal.replace(metastr, values[i]);
    }
    return retVal;
}

void
SSHKnownHosts::checkConnectionToUnknownHost(const bool newHost,
                                            SshSocket &ssh,
                                            LIBSSH2_SESSION *sshSession,
                                            libssh2_knownhost *hostInfo,
                                            bool& cancel) {
    const QString& msg = (newHost ? NewHostMessage : HostInfoChangedMessage);
    cancel = !checkUpdateKnownHosts(ssh, sshSession, msg, hostInfo);
}

bool
SSHKnownHosts::checkUpdateKnownHosts(SshSocket &ssh,
                                     LIBSSH2_SESSION *sshSession,
                                     const QString &msgFormat,
                                     libssh2_knownhost* hostInfo) {
    // Gather information required to populate meta-fields in the message
    const QString currKey =
        toHex(libssh2_hostkey_hash(sshSession, LIBSSH2_HOSTKEY_HASH_SHA1));
    const QString oldKey = (hostInfo != NULL) ? toHex(hostInfo->key) : "";
    // Create the list of values that can be used in messages in a fixed
    // order.
    QStringList values;
    values << ssh.peerName() << ssh.peerAddress().toString()
           << QString::number(ssh.peerPort()) << knownHostsFileName
           << currKey << oldKey << ssh.getReason();
    // Obtain the formatted message for display purposes
    const QString message = format(msgFormat, values);
    const QString details = format(ConnectDetailsMessage, values);
    // Create a message box and display information to the user.
    //QMessageBox msgBox(ssh.getOwner());
    QMessageBox msgBox;
    msgBox.setWindowTitle("Continue connecting with host?");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(message);
    msgBox.setInformativeText(ConnectQuestionMessage);
    msgBox.setDetailedText(details);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    // Show the message box and based on user's choice perform suitable action
    if (msgBox.exec() == QMessageBox::Yes) {
        // Add/update the new host info to the list.
        return addKnownHost(ssh, sshSession, hostInfo);
    }
    return false;
}

bool
SSHKnownHosts::addKnownHost(SshSocket& ssh, LIBSSH2_SESSION* sshSession,
                            libssh2_knownhost* currEntry) {
    if (currEntry != NULL) {
        // Delete the current entry (if any)
        libssh2_knownhost_del(knownHostsList, currEntry);
    }
    // Add entry for the current host in the session to the known hosts
    // First, Get the SSH fingerprint to be added.
    size_t hostKeyLen   = 0;
    int hostKeyType     = -1;
    const char *hostKey = libssh2_session_hostkey(sshSession, &hostKeyLen,
                                                  &hostKeyType);
    // Get the host name as C-string to pass to libSSH2
    const char *hostName = ssh.peerName().toStdString().c_str();
    const int keyTypeMask = LIBSSH2_KNOWNHOST_TYPE_PLAIN |
            ((hostKeyType == LIBSSH2_HOSTKEY_TYPE_RSA) ?
                 LIBSSH2_KNOWNHOST_KEY_SSHRSA : LIBSSH2_KNOWNHOST_KEY_SSHDSS);
    // Add the entry and check for successful completion of operation
    if (libssh2_knownhost_addc(knownHostsList, hostName, NULL, hostKey,
                               hostKeyLen, NULL, 0, keyTypeMask, NULL) != 0) {
        // Error adding data to in-memory list. This is not normal.
        char *errmsg; int msgLen;
        libssh2_session_last_error(sshSession, &errmsg, &msgLen, 0);
        qDebug() << errmsg;
        QMessageBox::critical(ssh.getOwner(), SshSocket::Title,
                              KnownHostLoadErrorMessage.arg(knownHostsFileName));
        return false;
    }
    // Save the newly added known host back to data file.
    const char* hostsFileName = knownHostsFileName.toStdString().c_str();
    if (libssh2_knownhost_writefile(knownHostsList, hostsFileName,
                                    LIBSSH2_KNOWNHOST_FILE_OPENSSH) != 0) {
        // Error writing known hosts data to file. This is not normal.
        QMessageBox::critical(ssh.getOwner(), SshSocket::Title,
                              UnableToSaveKnownHostsMessage.arg(knownHostsFileName));
        return false;
    }
    // Everything went well. Host is in the known hosts list.
    return true;
}

#endif
