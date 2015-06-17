#ifndef MUSE_APPLICATION_DATA_CPP
#define MUSE_APPLICATION_DATA_CPP

#include "MUSEApplicationData.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

namespace muse {
namespace appdata {

namespace {
    const QString knownHostsFileName = "known_hosts";
    const QString workspacesFileName = "workspaces";
}

bool firstRun() {
    QFileInfo knownHostsFile(knownHostsFilePath());
    QFileInfo workspacesFile(workspacesFilePath());

    return !knownHostsFile.exists() || !workspacesFile.exists();
}

void init() {
    QFile knownHostsFile(knownHostsFilePath());
    QFile workspacesFile(workspacesFilePath());

    if (!knownHostsFile.exists() && !knownHostsFile.open(QFile::WriteOnly)) {
        throw QString("Failed to create known hosts file");
    }

    if (!workspacesFile.exists() && !workspacesFile.open(QFile::WriteOnly)) {
        throw QString("Failed to create workspaces file");
    }
}

QString appDir() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString knownHostsFilePath() {
    return appDir() + QDir::separator() + knownHostsFileName;
}

QString workspacesFilePath() {
    return appDir() + QDir::separator() + workspacesFileName;
}

std::vector<QString> workspaces() {
    std::vector<QString> ret;

    QFile in(workspacesFilePath());
    in.open(QIODevice::ReadOnly | QIODevice::Text);

    while (!in.atEnd()) {
        ret.push_back(QString(in.readLine()));
    }

    return ret;
}

void addWorkspace(QString dir) {
    QFile out(workspacesFilePath());
    out.open(QIODevice::Append | QIODevice::Text);

    QTextStream stream(&out);

    stream << dir;// << std::endl;
}

}
}

#endif
