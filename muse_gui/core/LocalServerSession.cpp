#include "Core.h"
#include "LocalServerSession.h"
#include <QDir>
#include <QProcess>

LocalServerSession::LocalServerSession(Server &server, QWidget *parent, QString purpose) :
    ServerSession(server, parent), purpose(purpose) {
}


void
LocalServerSession::connectToServer() {

}


void
LocalServerSession::disconnectFromServer() {

}

int
LocalServerSession::exec(const QString &command, QString &stdoutput,  QString &stderrmsgs) {
    QProcess process; //= new QProcess();
    process.start(command);
    process.waitForFinished();
    stdoutput = process.readAllStandardOutput();
    stderrmsgs = process.readAllStandardError();

    return process.exitCode();

}

int
LocalServerSession::exec(const QString &command, QTextEdit& output) {

    QProcess process; //= new QProcess();
    process.start(command);
    process.waitForFinished();
    output.append(process.readAllStandardOutput());
    output.append(process.readAllStandardError());

    return process.exitCode();
}

void
LocalServerSession::copy(const QString& srcData, const QString &destDirectory,
                              const QString &destFileName, const int &mode) {
    QString newFilePath = destDirectory +
            (destDirectory.endsWith("/") ? destFileName : "/" + destFileName);
    QFile::copy(srcData, newFilePath);
    Q_UNUSED(mode);
}

void
LocalServerSession::copy(const QString& destData, const QString &srcDirectory,
                              const QString& srcFileName) {

    QString curFilePath = srcDirectory +
            (srcDirectory.endsWith("/") ? srcFileName : "/" + srcFileName);
    QFile::copy(curFilePath, destData);
}

void
LocalServerSession::mkdir(const QString &directory) {
    QDir dir(directory);
    emit directoryCreated(dir.mkdir(directory));
}

void
LocalServerSession::rmdir(const QString &directory) {
    QDir dir(directory);
    emit directoryRemoved(dir.rmdir(directory));
}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path) {

//}

void
LocalServerSession::setPurpose(const QString &text) {
    purpose = text;
}

void
LocalServerSession::setPerms(QFile &file, const char &permDigit, const bool owner) {
    QString perm = &permDigit;
    uint permNum = perm.toUInt(0, 16);
    file.setPermissions(file.symLinkTarget(), QFile::Permissions((permNum & 0x4)
                                                                 | (permNum & 0x2) |
                                                                 (permNum & 0x1)));

    Q_UNUSED(owner);
}
