#include "SshException.h"
#include "SshSocket.h"

#include <QMessageBox>

// Generic error message reported to the user.
const QString SshException::GenericErrorMessage =
        "An error occurred when creating or using an SSH socket. " \
        "The message above provides some information about the "   \
        "source of the error. Please try to resolve the source "   \
        "of the error and try the operation again.";

// Additional information about source of the exception
const QString SshException::ErrorDetails =
        "The exception was thrown in %1:%2\n"\
        "In method named   : %3\n" \
        "Socket  error code: %5\n" \
        "libSSH2 error code: %4";

SshException::SshException(SshSocket& ssh, const QString& msg,
                           int sshErrorCode,
                           QAbstractSocket::SocketError networkErrorCode,
                           const QString& fileName, const int lineNumber,
                           const QString& methodName) throw () : ssh(ssh),
    msg(msg), sshErrorCode(sshErrorCode), networkErrorCode(networkErrorCode),
    fileName(fileName), lineNumber(lineNumber), methodName(methodName) {
    // Nothing else to be done for now.
}

void
SshException::show(const SshException &exp, QWidget* parent) {
    if (parent == NULL) {
        parent = exp.ssh.getOwner();
    }
    QMessageBox msgBox(exp.ssh.getOwner());
    msgBox.setWindowTitle("SSH connectivity error");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(exp.msg);
    msgBox.setInformativeText(GenericErrorMessage);
    const QString exceptionDetails =
            ErrorDetails.arg(exp.fileName, QString::number(exp.lineNumber),
                             exp.methodName, QString::number(exp.sshErrorCode),
                             QString::number(exp.networkErrorCode));
    msgBox.setDetailedText(exceptionDetails);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}
