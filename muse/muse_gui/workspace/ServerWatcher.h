#ifndef SERVER_WATCHER_H
#define SERVER_WATCHER_H

#include <QThread>
#include <QString>
#include <QVector>

#include <atomic>

#include "Server.h"
#include "ServerList.h"

class ServerWatcher : public QThread {
    Q_OBJECT
public:
    ServerWatcher() noexcept;

    void forceUpdate();

signals:
    void errorEncountered(Server server, QString message);

private:
    void run() override;

    void update();

    void processAllServers(ServerList& servers);

    void processRemoteServers(ServerList& servers);
    void processLocalServers(ServerList& servers);

    void processRemoteServer(Server& server);
    void processLocalServer(Server& server);

    std::atomic<bool> needsUpdate;

    //ServerList& servers;
};

#endif // SERVER_WATCHER_H
