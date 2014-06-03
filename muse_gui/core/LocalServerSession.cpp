#include "Core.h"
#include "LocalServerSession.h"
#include <libssh2.h>

LocalServerSession::LocalServerSession(Server &server, QWidget *parent) : ServerSession(server, parent) {
    Q_UNUSED(server);
    Q_UNUSED(parent);
}


void LocalServerSession::connect() {

}


void LocalServerSession::disconnect() {

}

int LocalServerSession::exec(const QString &command, QString &stdoutput,  QString &stderrmsgs) {
    Q_UNUSED(command);
    Q_UNUSED(stdoutput);
    Q_UNUSED(stderrmsgs);

}

int LocalServerSession::exec(const QString &command, QTextDocument &output) {

    Q_UNUSED(command);
    Q_UNUSED(output);
}

//Once libssh2 implementation is figured out....
//LocalServerSession::startProcess(){}

void LocalServerSession::copy(std::istream &srcData, const QString &destDirectory,
                              const QString &destFileName, const QString &mode) {

    Q_UNUSED(srcData);
    Q_UNUSED(destDirectory);
    Q_UNUSED(destFileName);
    Q_UNUSED(mode);
}

void LocalServerSession::copy(std::ostream &destData, const QString &srcDirectory,
                              const QString &srcFileName) {

    Q_UNUSED(destData);
    Q_UNUSED(srcDirectory);
    Q_UNUSED(srcFileName);
}

void LocalServerSession::mkdir(const QString &directory) {

    Q_UNUSED(directory);
}

void LocalServerSession::rmdir(const QString &directory) {

    Q_UNUSED(directory);
}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path) {

//}

void LocalServerSession::setPurpose(const QString &text) {

    Q_UNUSED(text);
}

void LocalServerSession::setPerms(File &file, const char &permDigit, const bool owner){

    Q_UNUSED(file);
    Q_UNUSED(permDigit);
    Q_UNUSED(owner);
}
