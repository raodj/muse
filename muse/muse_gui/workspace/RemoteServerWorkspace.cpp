#ifndef REMOTE_SERVER_WORKSPACE_CPP
#define REMOTE_SERVER_WORKSPACE_CPP

#include "RemoteServerWorkspace.h"
#include "Workspace.h"
#include "XMLParser.h"

#include <QFileInfo>

RemoteServerWorkspace::RemoteServerWorkspace(QString dir, const SshSocket &socket)
    : ServerWorkspace(dir), channel{ socket }
{
}

QString
RemoteServerWorkspace::save() {
    // Save our data to a temporary QString
    QString schemaFile{ ":/resources/muse_gui.xsd" };
    QString serverData;
    QString result{ XMLParser{}.saveXML(serverData, schemaFile, *this) };

    if (result != "") {
        return result;
    }

    if (!channel.copy(serverData, directory, serverFileName, 0600)) {
        return QString{ "Failed to write the XML data to the remote server" };
    }

    return "";
}

QString
RemoteServerWorkspace::load() {
    // Because of the way the XMLParser is set up, it can only load data from
    // a local XML file, so what we will do is copy the XML file from the remote
    // server into our local Workspace (the workspace for MUSE not this server),
    // parse that temporary file with XMLParser, then delete that file

    Workspace* ws = Workspace::get();
    QFileInfo tempFileInfo{ QDir{ ws->getDirectory() }, serverFileName };
    QFile tempFile{ tempFileInfo.absoluteFilePath() };

    // Create the temp xml file if it doesnt exist
    if (!tempFile.open(QIODevice::ReadWrite)) {
        return QString{ "Failed to create temporary XML file for loading "\
                        "from the remote server" };
    }

    // Copy the data from the servers xml file to a temporary QString
    QString tempData;
    bool result = channel.copy(tempData, directory, serverFileName);
    if (!result) {
        return QString{ "Failed to read the XML from the server" };
    }

    // Write the data we just read from the server to our temp file
    QTextStream tempFileOut{ &tempFile };
    tempFileOut << tempData;

    // Load our temporary xml file with the XMLParser then remove the file
    QString schemaFile{ ":/resources/muse_gui.xsd" };
    result = XMLParser{}.loadXML(tempFileInfo.absoluteFilePath(), schemaFile, *this);
    tempFile.remove();

    return result;
}

#endif
