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

void
LocalServerWorkspace::save() {
    QFileInfo info{ QDir{ directory }, serverFileName };
    XMLParser parser;
    QString error{ parser.saveXML(info.absoluteFilePath(), *this) };

    if (error != "") {
        throw error;
    }
}

void
LocalServerWorkspace::load() {
    QDir serverDir{ directory };
    QFileInfo info{ serverDir, serverFileName };

    if (!info.exists() || !info.isFile() || !info.isReadable()) {
        throw QString{ "Server directory ('" + directory + "') does not "\
                       "contain a readable metadata file: " + serverFileName };
    }

    XMLParser parser;
    QString schemaFile{ ":/resources/muse_gui.xsd" };
    QString error;

    error = parser.loadXML(info.absoluteFilePath(), schemaFile, *this);
    if (error != "") {
        throw error;
    }
}

#endif
