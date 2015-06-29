#include <iostream>

#include "ServerWatcher.h"
#include "Workspace.h"
#include "ServerList.h"

ServerWatcher::ServerWatcher() noexcept {
}

void
ServerWatcher::forceUpdate() {
}

void
ServerWatcher::run() {
    while (true) {
        update();

        QThread::sleep(10);
    }
}

void
ServerWatcher::update() {
    if (Workspace::get() == nullptr) {
        return;
    }

    processAllServers(Workspace::get()->getServerList());
}

void
ServerWatcher::processAllServers(ServerList& servers) {
    for (int i = 0; i < servers.size(); i++) {
        if (servers.get(i).isRemote()) {
            processRemoteServer(servers.get(i));
        } else {
            processLocalServer(servers.get(i));
        }
    }
}

void
ServerWatcher::processRemoteServers(ServerList& servers) {
    for (int i = 0; i < servers.size(); i++) {
        if (servers.get(i).isRemote()) {
            processRemoteServer(servers.get(i));
        }
    }
}

void
ServerWatcher::processLocalServers(ServerList& servers) {
    for (int i = 0; i < servers.size(); i++) {
        if (!servers.get(i).isRemote()) {
            processRemoteServer(servers.get(i));
        }
    }
}

void
ServerWatcher::processRemoteServer(Server& server) {
    Q_ASSERT(server.isRemote());

    std::cout << "* Remote Server" << std::endl;
    std::cout << "* name: " << server.getName().toStdString() << std::endl;
    std::cout << "* remote: " << std::boolalpha << server.isRemote() << std::endl;
    std::cout << "* id: " << server.getID().toStdString() << std::endl;
}

void
ServerWatcher::processLocalServer(Server& server) {
    Q_ASSERT(!server.isRemote());

    std::cout << "* Local Server" << std::endl;
    std::cout << "* name: " << server.getName().toStdString() << std::endl;
    std::cout << "* remote: " << std::boolalpha << server.isRemote() << std::endl;
    std::cout << "* id: " << server.getID().toStdString() << std::endl;
}
