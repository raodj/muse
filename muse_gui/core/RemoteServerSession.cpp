#include "RemoteServerSession.h"

RemoteServerSession::RemoteServerSession(Server *server, QWidget *parent):ServerSession(server, parent){

}

void RemoteServerSession::connect(){

}

void RemoteServerSession::disconnect(){

}

void RemoteServerSession::getPassword(){

}

int RemoteServerSession::exec(QString *command, QString *outputs){

}

int RemoteServerSession::exec(QString *command, QTextDocument *output){

}

bool RemoteServerSession::verifyServerHostKey(QString *hostName, int port, QString *serverHostKeyAlgorithm, char *serverHostKey){

}

void RemoteServerSession::addKnownHost(QString *hostName, int port, QString *serverHostKeyAlgorithm, char *serverHostKey){

}

void RemoteServerSession::loadKnownHosts(){

}

void RemoteServerSession::copy(std::istream *srcData, QString *destDirectory, QString *destFileName, QString *mode){

}

void RemoteServerSession::copy(std::ostream *destData, QString *srcDirectory, QString *srcFileName){

}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path){

//}

void RemoteServerSession::mkdir(QString *directory){

}

void RemoteServerSession::rmdir(QString *directory){

}

void RemoteServerSession::setPurpose(QString *text){

}
