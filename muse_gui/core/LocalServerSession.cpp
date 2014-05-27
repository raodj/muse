#include "Core.h"
#include "LocalServerSession.h"
//#include <libssh2.h>

LocalServerSession::LocalServerSession(Server *server, QWidget *parent) :ServerSession(server, parent){

}


void LocalServerSession::connect(){

}


void LocalServerSession::disconnect(){

}



int LocalServerSession::exec(QString *command, QString *outputs){

}

int LocalServerSession::exec(QString *command, QTextDocument *output){

}

//Once libssh2 implementation is figured out....
//LocalServerSession::startProcess(){}

void LocalServerSession::copy(std::istream *srcData, QString *destDirectory, QString *destFileName, QString *mode){

}

void LocalServerSession::copy(std::ostream *destData, QString *srcDirectory, QString *srcFileName){

}

void LocalServerSession::mkdir(QString *directory){

}

void LocalServerSession::rmdir(QString *directory){

}

//To be reincluded into the class definition when the FileInfo class
//gets redefined

//FileInfo LocalServerSession::fStat(QString *path){

//}

void LocalServerSession::setPurpose(QString *text){

}

void LocalServerSession::setPerms(File *file, char permDigit, bool owner){

}
