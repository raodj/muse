

#ifndef SERVERSESSION_H
#define SERVERSESSION_H
#include <iostream>
#include <ostream>

class QString;
class QTextDocument;
class Server;
class QWidget;
class FileInfo;

class ServerSession{
public:
    ServerSession(Server *server, QWidget *parent);
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual int exec(QString *command, QString *outputs) = 0;
    virtual int exec(QString *command, QTextDocument *output) = 0;
    virtual void copy(std::istream *srcData, QString *destDirectory, QString destFileName, QString *mode) = 0;

    //Java version of below method also had a progress bar as a last parameter.....
    virtual void copy(std::ostream *destData, QString *srcDirectory, QString srcFileName) = 0;

    virtual void mkdir(QString *directory) = 0;
    virtual void rmdir(QString *directory) = 0;

    //FileInfo is a PEACE class that has yet to be translated to C++
    virtual FileInfo fstat(QString *path) = 0;

    /*Server class not yet translated to C++, so this method header may change
    once the Server class exists and the enumerations are transferred. Maybe it should
   return an int? */
    //virtual Server.OSType getOSType()= 0;

    virtual void setPurpose(QString *text) = 0;


protected:
    const Server *server;
    const QWidget *parent;

};

#endif // SERVERSESSION_H
