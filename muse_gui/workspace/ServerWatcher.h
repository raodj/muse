#ifndef SERVER_WATCHER_H
#define SERVER_WATCHER_H

#include <QThread>
#include <QString>
#include <QVector>

#include <atomic>

#include "Server.h"

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

    void processRemoteServers();
    void processLocalServers();

    void processRemoteServer(Server server);
    void processLocalServer(Server server);

    std::vector<Server> getRemoteServers();
    std::vector<Server> getLocalServers();

    std::atomic<bool> needsUpdate;
};

#endif // SERVER_WATCHER_H
