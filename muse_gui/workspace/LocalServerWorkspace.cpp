#ifndef LOCAL_SERVER_WORKSPACE_CPP
#define LOCAL_SERVER_WORKSPACE_CPP

#include "LocalServerWorkspace.h"
#include "XMLParser.h"

#include <QFileInfo>
#include <QDir>

LocalServerWorkspace::LocalServerWorkspace(const QString& dir)
    : ServerWorkspace(dir)
{
}

LocalServerWorkspace::~LocalServerWorkspace() {

}

QString
LocalServerWorkspace::save() {
    QFileInfo info{ QDir{ directory }, serverFileName };

    return XMLParser{}.saveXML(info.absoluteFilePath(), *this);
}

QString
LocalServerWorkspace::load() {
    QDir serverDir{ directory };
    QFileInfo info{ serverDir, serverFileName };

    if (!info.exists() || !info.isFile() || !info.isReadable()) {
        return QString{ "Server directory ('" + directory + "') does not "\
                        "contain a readable metadata file: " + serverFileName };
    }

    QString schemaFile{ ":/resources/muse_gui.xsd" };

    return XMLParser{}.loadXML(info.absoluteFilePath(), schemaFile, *this);
}

#endif
