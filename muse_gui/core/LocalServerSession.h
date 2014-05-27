

#ifndef LOCALSERVERSESSION_H
#define LOCALSERVERSESSION_H

#include "ServerSession.h"
#include <stdio.h>
class File;
class LocalServerSession : public ServerSession
{
public:
    void connect();
    void disconnect();
    int exec(QString *command, QString *outputs);
    int exec(QString *command, QTextDocument *output);
    //Maybe this handles the enumeration??
    //int getOSType();
    void copy(std::istream *srcData, QString *destDirectory, QString *destFileName, QString *mode);

    //Java version of below method also had a progress bar as a last parameter.....
    void copy(std::ostream *destData, QString *srcDirectory, QString *srcFileName);

    void mkdir (QString *directory);
    void rmdir(QString *directory);
    FileInfo fStat(QString *path);
    void setPurpose(QString *text);
    QString* getPurpose();


protected:
    LocalServerSession(Server *server, QWidget *parent);

private:
    QString *purpose;
    void setPerms(File *file, char permDigit, bool owner);

    //Not sure on this yet....
//    Process startProcess(QString *command);
};

#endif // LOCALSERVERSESSION_H
