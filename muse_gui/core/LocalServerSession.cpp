#include "Core.h"
#include "LocalServerSession.h"
#include <QDir>
#include <QProcess>

LocalServerSession::LocalServerSession(Server &server, QWidget *parent, QString purpose) :
    ServerSession(server, parent), purpose(purpose) {
    Q_UNUSED(server);
    Q_UNUSED(parent);
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

    Q_UNUSED(command);
    Q_UNUSED(output);
}

//Once libssh2 implementation is figured out....
//LocalServerSession::startProcess(){}

void
LocalServerSession::copy(const QString& srcData, const QString &destDirectory,
                              const QString &destFileName, const QString &mode) {

    Q_UNUSED(srcData);
    Q_UNUSED(destDirectory);
    Q_UNUSED(destFileName);
    Q_UNUSED(mode);
}

void
LocalServerSession::copy(const QString& destData, const QString &srcDirectory,
                              const QString &srcFileName) {

    Q_UNUSED(destData);
    Q_UNUSED(srcDirectory);
    Q_UNUSED(srcFileName);
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

    Q_UNUSED(text);
}

void
LocalServerSession::setPerms(QFile &file, const char &permDigit, const bool owner){

    Q_UNUSED(file);
    Q_UNUSED(permDigit);
    Q_UNUSED(owner);
}
