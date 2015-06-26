#include <iostream>

#include "ServerWatcher.h"
#include "Workspace.h"

ServerWatcher::ServerWatcher() noexcept {
}

void
ServerWatcher::forceUpdate() {
}

void
ServerWatcher::run() {
    while (true) {
        // check the servers for updates
        update();

        QThread::sleep(10);
    }
}

void
ServerWatcher::update() {
    std::cout << "*** Local Servers ***" << std::endl;
    processLocalServers();
    std::cout << "*** End Local Servers ***" << std::endl;

    std::cout << "*** Remote Servers ***" << std::endl;
    processRemoteServers();
    std::cout << "*** End Remote Servers ***" << std::endl;
}

void
ServerWatcher::processRemoteServers() {
    for (auto s : getRemoteServers()) {
        processRemoteServer(s);
    }
}

void
ServerWatcher::processLocalServers() {
    for (auto s : getLocalServers()) {
        processLocalServer(s);
    }
}

void
ServerWatcher::processRemoteServer(Server server) {
    std::cout << "* Remote Server" << std::endl;
    std::cout << "* name: " << server.getName().toStdString() << std::endl;
    std::cout << "* remote: " << std::boolalpha << server.isRemote() << std::endl;
    std::cout << "* id: " << server.getID().toStdString() << std::endl;
}

void
ServerWatcher::processLocalServer(Server server) {
    std::cout << "* Local Server" << std::endl;
    std::cout << "* name: " << server.getName().toStdString() << std::endl;
    std::cout << "* remote: " << std::boolalpha << server.isRemote() << std::endl;
    std::cout << "* id: " << server.getID().toStdString() << std::endl;
}

std::vector<Server>
ServerWatcher::getRemoteServers() {
    std::vector<Server> ret;

    Workspace* workspace = Workspace::get();

    if (workspace != nullptr) {
        ret.push_back(Server("0", true, "remote0"));
        ret.push_back(Server("1", true, "remote1"));
        ret.push_back(Server("2", true, "remote2"));
    }

    return ret;
}

std::vector<Server>
ServerWatcher::getLocalServers() {
    std::vector<Server> ret;

    ret.push_back(Server("0", false, "local0"));
    ret.push_back(Server("1", false, "local1"));
    ret.push_back(Server("2", false, "local2"));

    return ret;
}
