#ifndef MUSE_APPLICATION_DATA_H
#define MUSE_APPLICATION_DATA_H

#include <QString>

#include <vector>

namespace muse {
namespace appdata {

bool firstRun();

void init();

QString appDir();

QString knownHostsFilePath();
QString workspacesFilePath();

std::vector<QString> workspaces();

void addWorkspace(QString dir);

}
}

#endif // MUSE_APPLICATION_DATA_H
