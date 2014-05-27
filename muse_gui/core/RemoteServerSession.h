#ifndef REMOTESERVERSESSION_H
#define REMOTESERVERSESSION_H

#include "ServerSession.h"

class RemoteServerSession : public ServerSession
{
public:
    void connect();
    void disconnect();
    int exec(QString *command, QString *outputs);
    int exec(QString *command, QTextDocument *output);
    bool verifyServerHostKey(QString *hostName, int port, QString *serverHostKeyAlgorithm, char *serverHostKey);
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
    RemoteServerSession(Server *server, QWidget *parent);

private:
    void getPassword();
    void addKnownHost(QString *hostName, int port, QString *serverHostKeyAlgorithm, char *serverHostKey);
    void loadKnownHosts();
    /*Set of variables yet to be implemented...
    Connection connection;
    Server.OSType osType;
    static Object knownHostsLock;
    static KnownHosts knownHosts;
    */
    QString *purpose;
    const QString *KNOWN_HOST_PATH;

};

#endif // REMOTESERVERSESSION_H
