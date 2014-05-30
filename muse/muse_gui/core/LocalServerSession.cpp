#include "Core.h"
#include "LocalServerSession.h"
//#include <libssh2.h>

LocalServerSession::LocalServerSession(Server &server, QWidget *parent) :ServerSession(server, parent){

}


void LocalServerSession::connect(){

}


void LocalServerSession::disconnect(){

}

int LocalServerSession::exec(const QString &command, QString &stdoutput,  QString &stderrmsgs){

}

int LocalServerSession::exec(const QString &command, QTextDocument &output){

}

//Once libssh2 implementation is figured out....
//LocalServerSession::startProcess(){}

void LocalServerSession::copy(std::istream &srcData, const QString &destDirectory, const QString &destFileName, const QString &mode){

}

void LocalServerSession::copy(std::ostream &destData, const QString &srcDirectory, const QString &srcFileName){

}

void LocalServerSession::mkdir(const QString &directory){

}

void LocalServerSession::rmdir(const QString &directory){

}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path){

//}

void LocalServerSession::setPurpose(const QString &text){

}

void LocalServerSession::setPerms(File &file, const char &permDigit, const bool owner){

}
