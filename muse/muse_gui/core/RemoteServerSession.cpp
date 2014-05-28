#include "Core.h"
#include "RemoteServerSession.h"

RemoteServerSession::RemoteServerSession(Server &server, QWidget *parent):ServerSession(server, parent){

}

void RemoteServerSession::connect(){

}

void RemoteServerSession::disconnect(){

}

void RemoteServerSession::getPassword(){

}

int RemoteServerSession::exec(const QString &command, QString &stdoutput, QString &stderrmsgs){

}

int RemoteServerSession::exec(const QString &command, QTextDocument &output){

}

bool RemoteServerSession::verifyServerHostKey(const QString &hostName, const int port,
                                              const QString &serverHostKeyAlgorithm,
                                              const char &serverHostKey){

}

void RemoteServerSession::addKnownHost(const QString &hostName, const int port,
                                       const QString &serverHostKeyAlgorithm,
                                       const char &serverHostKey){

}

void RemoteServerSession::loadKnownHosts(){

}

void RemoteServerSession::copy(std::istream &srcData, const QString &destDirectory,
                               const QString &destFileName, const QString &mode){

}

void RemoteServerSession::copy(std::ostream &destData, const QString &srcDirectory, const QString &srcFileName){

}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path){

//}

void RemoteServerSession::mkdir(const QString &directory){

}

void RemoteServerSession::rmdir(const QString &directory){

}

void RemoteServerSession::setPurpose(const QString &text){

}
