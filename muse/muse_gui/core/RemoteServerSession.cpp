#include "Core.h"
#include "RemoteServerSession.h"

RemoteServerSession::RemoteServerSession(Server &server, QWidget *parent) :
    ServerSession(server, parent) {

    Q_UNUSED(server);
    Q_UNUSED(parent);
}

void RemoteServerSession::connect() {

}

void RemoteServerSession::disconnect() {

}

void RemoteServerSession::getPassword() {

}

int RemoteServerSession::exec(const QString &command, QString &stdoutput,
                              QString &stderrmsgs) {
    Q_UNUSED(command);
    Q_UNUSED(stdoutput);
    Q_UNUSED(stderrmsgs);

}

int RemoteServerSession::exec(const QString &command, QTextDocument &output) {

    Q_UNUSED(command);
    Q_UNUSED(output);
}

bool RemoteServerSession::verifyServerHostKey(const QString &hostName, const int port,
                                              const QString &serverHostKeyAlgorithm,
                                              const char &serverHostKey) {
    Q_UNUSED(hostName);
    Q_UNUSED(port);
    Q_UNUSED(serverHostKey);
    Q_UNUSED(serverHostKeyAlgorithm);

}

void RemoteServerSession::addKnownHost(const QString &hostName, const int port,
                                       const QString &serverHostKeyAlgorithm,
                                       const char &serverHostKey) {
    Q_UNUSED(hostName);
    Q_UNUSED(port);
    Q_UNUSED(serverHostKeyAlgorithm);
    Q_UNUSED(serverHostKey);

}

void RemoteServerSession::loadKnownHosts() {

}

void RemoteServerSession::copy(std::istream &srcData, const QString &destDirectory,
                               const QString &destFileName, const QString &mode) {
    Q_UNUSED(srcData);
    Q_UNUSED(destDirectory);
    Q_UNUSED(destFileName);
    Q_UNUSED(mode);

}

void RemoteServerSession::copy(std::ostream &destData, const QString &srcDirectory,
                               const QString &srcFileName) {
    Q_UNUSED(destData);
    Q_UNUSED(srcDirectory);
    Q_UNUSED(srcFileName);


}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path){

//}

void RemoteServerSession::mkdir(const QString &directory) {
    Q_UNUSED(directory);

}

void RemoteServerSession::rmdir(const QString &directory) {
    Q_UNUSED(directory);

}

void RemoteServerSession::setPurpose(const QString &text) {
    Q_UNUSED(text);

}
