#ifndef LOCAL_SERVER_WORKSPACE_CPP
#define LOCAL_SERVER_WORKSPACE_CPP

#include "LocalServerWorkspace.h"
#include "XMLParser.h"

#include <QFileInfo>
#include <QDir>

LocalServerWorkspace::LocalServerWorkspace(QString dir)
    : ServerWorkspace(dir)
{
}

QString
LocalServerWorkspace::save() {
    QFileInfo info{ QDir{ directory }, serverFileName };

    return XMLParser{}.saveXML(info.absoluteFilePath(), *this);
}

QString
LocalServerWorkspace::load() {
    QFileInfo info{ QDir{ directory }, serverFileName };

    if (!info.exists() || !info.isFile() || !info.isReadable()) {
        return QString{ "Server directory ('" + directory + "') does not "\
                        "contain a readable metadata file: " + serverFileName };
    }

    QString schemaFile{ ":/resources/muse_gui.xsd" };

    return XMLParser{}.loadXML(info.absoluteFilePath(), schemaFile, *this);
}

#endif
