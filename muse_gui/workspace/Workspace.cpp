#ifndef WORKSPACE_CPP
#define WORKSPACE_CPP

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "Workspace.h"
#include "XMLParser.h"
#include "Logger.h"
#include "Core.h"
#include <QDir>
#include <QFileInfo>

// The singleton workspace object
Workspace* Workspace::workspace = NULL;

// The actual XML workspace file name
const QString Workspace::WorkspaceFileName  = "MUSEWorkspace.xml";

Workspace::Workspace(const QString& dir, bool isValid) :
    XMLRootElement("Workspace"), directory(dir),
    timestamp(QDateTime::currentDateTime()), seqCounter(0), isGood(isValid) {
    // Register all the sub-elements for unmarshalling
    registerClasses();
    // Add default namespace definitions
    const QString DefNS = "http://pc2lab.cec.miamiOH.edu/";
    const QString XsiNS = "http://www.w3.org/2001/XMLSchema-instance";
    namespaces.append(QXmlStreamNamespaceDeclaration("", DefNS));
    namespaces.append(QXmlStreamNamespaceDeclaration("xsi", XsiNS));
    // Add default top-level attributes
    const QString schemaLoc = "http://www.peace-tools.org/ muse_gui.xsd";
    attributes.append("xsi:schemaLocation", schemaLoc);
    attributes.append("Version",        "0.1");
    // Add the top-level elements to the base class.
    addElement(XMLElementInfo("Directory",  &directory));
    addElement(XMLElementInfo("Timestamp",  &timestamp));
    addElement(XMLElementInfo("SeqCounter", &seqCounter));
    addElement(XMLElementInfo("ServerList", &serverList));
}

void
Workspace::registerClasses() {
    // Register all the underlying sub-elements for unmarshalling
    qRegisterMetaType<ServerList>("ServerList");
    qRegisterMetaType<Server>("Server");
}

void
Workspace::dumpWorkspace() {
    XMLParser xmlParser;
    QString wsInfo;
    xmlParser.saveXML(wsInfo, *this);
    progLog() << endl << wsInfo;
}

QString
Workspace::createWorkspace(const QString &directory) {
    // Ensure that the directory exists and writable
    QDir dir(directory);
    if (!dir.exists() || !dir.isReadable()) {
        // Directory does not exists or it is not readable.
        return "Workspace directory ('" + directory + "') does not exist " +
                "or is not readable.";
    }
    // Clear out existing workspace (if any)
    if (workspace != NULL) {
        delete workspace;
    }
    // Create a default workspace (assuming it is going to be usable)
    const QString wsDir = dir.absolutePath() + QDir::separator();
    workspace = new Workspace(wsDir, true);
    // Write the workspace to the workspace file in the given directory.
    const QString xmlFileName = wsDir + WorkspaceFileName;
    QString errMsg = "";
    XMLParser xmlParser;
    if ((errMsg = xmlParser.saveXML(xmlFileName, *workspace)) != "") {
        // Error occurred!
        workspace->isGood = false;
        return errMsg;
    }
    // Everything went well. Update the default workspace.
    workspace->isGood = true;
    // Write the loaded data to programmer logs for cross reference.
    workspace->dumpWorkspace();
    // Return success (no error message).
    return "";
}

// Load an existing workspace.
QString
Workspace::useWorkspace(const QString &directory) {
    // First check to ensure that the file actually exists.
    QFileInfo wsFile(QDir(directory), WorkspaceFileName);
    if (!wsFile.exists() || !wsFile.isFile() || !wsFile.isReadable()) {
        return "Workspace directory ('" + directory + "') does not " +
               "constain a readable metadata file " + WorkspaceFileName;
    }
    // Try and unmarshall the data in the workspace file into temporary ws.
    XMLParser xmlParser;
    Workspace *ws = new Workspace();
    QString   errMsg, schemaFile = ":/resources/muse_gui.xsd";
    if ((errMsg = xmlParser.loadXML(wsFile.filePath(),
                                    schemaFile, *ws)) != "") {
        // Error occurred. Do no further operations.
        delete ws;
        return errMsg;
    }
    // Workspace loaded successfully! Update reference to workspace
    if (workspace != NULL) {
        // Clear out existing workspace.
        delete workspace;
    }
    workspace         = ws;
    workspace->isGood = true; // Set the workspace is in good condition.
    // Write the loaded data to programmer logs for cross reference.
    workspace->dumpWorkspace();
    // Add the list of servers to the table model
    workspace->addInitialServersToModel();
    // Return success (no error message).
    return "";
}

QString
Workspace::saveWorkspace() {
    const QString filePath = directory + WorkspaceFileName;
    XMLParser saver;
    return saver.saveXML(filePath, *workspace);
}

QString
Workspace::reserveID(const QString& itemType) {
    return itemType + QString::number(seqCounter++, 10);
}

void
Workspace::addInitialServersToModel() {
    for (int i = 0; i < serverList.size(); i++) {
        serverModel.appendServerEntry(serverList.get(i));
    }
}

ServerListTableModel &
Workspace::getTableModel() {
    return serverModel;
}

void
Workspace::addServerToWorkSpace(Server &server) {
    serverList.addServer(server);
    serverModel.appendServerEntry(server);
}

#endif
