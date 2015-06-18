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
#include "Logger.h"
#include "Core.h"
#include "Server.h"
#include "ServerList.h"
#include "XMLRootElement.h"
#include "XMLParser.h"

#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDateTime>

//namespace muse {
//namespace workspace {

///*
// * the data about a workspace is kept in an xml file in the workspaces
// * directory, the WorkspaceMetadata class is completely contained in an
// * anonymous namesace so that it is invisible to the rest of the program,
// * only the muse::workspace namespace should actually know how a workspace
// * is handled internally
// */
//namespace {

//class WorkspaceMetadata : public XMLRootElement {
//public:
//    WorkspaceMetadata(QString dir) :
//        XMLRootElement("Workspace"), directory(dir),
//        timestamp(QDateTime::currentDateTime()), counter(0)
//    {
//        // register all the sub-elements for unmarshalling
//        registerClasses();

//        // add default namespace definitions
//        const QString DefNS = "http://pc2lab.cec.miamiOH.edu/";
//        const QString XsiNS = "http://www.w3.org/2001/XMLSchema-instance";
//        namespaces.append(QXmlStreamNamespaceDeclaration("", DefNS));
//        namespaces.append(QXmlStreamNamespaceDeclaration("xsi", XsiNS));

//        // add default top-level attributes
//        const QString schemaLoc = "http://www.peace-tools.org/ muse_gui.xsd";
//        attributes.append("xsi:schemaLocation", schemaLoc);
//        attributes.append("Version", "0.1");

//        // add the top-level elements to the base class.
//        addElement(XMLElementInfo("Directory",  &directory));
//        addElement(XMLElementInfo("Timestamp",  &timestamp));
//        addElement(XMLElementInfo("Counter", &counter));
//    }

//    void save() {

//    }

//private:
//    void registerClasses() {
//        // register all the underlying sub-elements for unmarshalling
//        qRegisterMetaType<ServerList>("ServerList");
//        qRegisterMetaType<Server>("Server");
//    }

//    QString directory;
//    QDateTime timestamp;

//    long int counter;

//    ServerList servers;
//};

//WorkspaceMetadata metadata;

//} // end anonymous namespace

//bool create(QString path) {
//    // TODO: test if a workspace already exists in this directory, if there is
//    // one, ask the user if they want to override it, for now, just assume there
//    // is no workspace here yet


//}

//void use(QString path) {

//}

///*
// * functions visible to the rest of muse gui
// */

//void init() {
//    timestamp = QDateTime::currentDateTime();
//    counter = 0;

//    if (checkForNeededFile()) {
//        useExisting();
//    } else {
//        createNew();
//    }

//    serverWatcher = std::thread(serverWatcherExec);
//}

//void save() {
////    Json::Value root = Json::objectValue;
////    Json::Value list = root["server-list"];
////    list = Json::arrayValue;

////    timestamp = QDateTime::currentDateTime();

////    root["directory"] = appDir().toStdString();
////    root["timestamp"] = timestamp.toString().toStdString();
////    root["counter"] = (int) counter;

////    for (auto& s : servers) {
////        list.append(s.save());
////    }

////    muse::json::save(saveDataFilePath(), root);
//}

//void load() {
////    Json::Value root = muse::json::load(saveDataFilePath());
////    Json::Value list = root["server-list"];

////    timestamp = QDateTime::fromString(QString::fromStdString(root["timestamp"].asString()));
////    counter = root["counter"].asInt();

////    for (auto& item : list) {
////        addServer(Server(item));
////    }

////    muse::json::save(saveDataFilePath(), root);
//}

//void clear() {
//    servers.clear();
//}

//void addServer(Server server) {
//    servers.push_back(server);
//}

//void removeServer(Server server) {
//    servers.erase(std::remove(std::begin(servers), std::end(servers), server), std::end(servers));
//}

//bool firstRun() {
//    return !checkForNeededFile();
//}

//std::vector<Server> getServers() {
//    return servers;
//}

//int serverCount() {
//    return servers.size();
//}

//Server getServer(int index) {
//    return servers.at(index);
//}

//QString appDir() {
//    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
//}

//QString knownHostsFilePath() {
//    return appDir() + QDir::separator() + knownHostsFileName;
//}

//QString saveDataFilePath() {
//    return appDir() + QDir::separator() + saveDataFileName;
//}

//QString reserveID(const QString &itemType) {
//    return itemType + QString::number(counter++);
//}

//} // end namespace workspace
//} // end namespace muse

// The singleton workspace object
Workspace* Workspace::workspace = NULL;

// The actual XML workspace file name
const QString Workspace::workspaceFileName  = "MUSEWorkspace.xml";

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
    attributes.append("Version", "0.1");

    // Add the top-level elements to the base class.
    addElement(XMLElementInfo("Directory",  &directory));
    addElement(XMLElementInfo("Timestamp",  &timestamp));
    addElement(XMLElementInfo("SeqCounter", &seqCounter));
    addElement(XMLElementInfo("ServerList", serverModel.getServerList()));
   // addElement(XMLElementInfo("JobList", &jobList));
}

void
Workspace::registerClasses() {
    // Register all the underlying sub-elements for unmarshalling
    qRegisterMetaType<ServerList>("ServerList");
    qRegisterMetaType<Server>("Server");
//    qRegisterMetaType<ProjectList>("ProjectList");
//    qRegisterMetaType<Project>("Project");
//    qRegisterMetaType<JobList>("JobList");
//    qRegisterMetaType<Job>("Job");
}

void
Workspace::dumpWorkspace() {
    XMLParser xmlParser;
    QString wsInfo;
    xmlParser.saveXML(wsInfo, *this);
    progLog() << endl << wsInfo;
}

bool
Workspace::isWorkspace(const QString &directory) {
    QFileInfo file(directory + QDir::separator() + workspaceFileName);

    return file.exists();
}

QString
Workspace::createWorkspace(const QString &directory) {
    // Clear out existing workspace (if any)
    if (workspace != NULL) {
        delete workspace;
    }

    QDir dir(directory);
    QString wsDir = dir.absolutePath() + QDir::separator();
    workspace = new Workspace(wsDir, true);

    // Write the workspace to the workspace file in the given directory.
    const QString xmlFileName = wsDir + workspaceFileName;
    QString errMsg;
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
    QFileInfo wsFile(QDir(directory), workspaceFileName);
    if (!wsFile.exists() || !wsFile.isFile() || !wsFile.isReadable()) {
        return "Workspace directory ('" + directory + "') does not " +
               "constain a readable metadata file " + workspaceFileName;
    }

    // Try and unmarshall the data in the workspace file into temporary ws.
    XMLParser xmlParser;
    QDir dir(directory);
    const QString wsDir = dir.absolutePath() + QDir::separator();
    Workspace *ws = new Workspace();
    QString errMsg;
    QString schemaFile = ":/resources/muse_gui.xsd";

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

    workspace = ws;
    workspace->isGood = true; // Set the workspace is in good condition.

    // Write the loaded data to programmer logs for cross reference.
    workspace->dumpWorkspace();

    // Add the list of servers to the table model
//    workspace->addInitialServersToModel();
//    workspace->addInitialProjectsToModel();
//    workspace->addInitialJobsToModel();

    // Return success (no error message).
    return "";
}

QString
Workspace::saveWorkspace() {
    const QString filePath = directory + QDir::separator() + workspaceFileName;
    XMLParser saver;

    return saver.saveXML(filePath, *workspace);
}

QString
Workspace::reserveID(const QString& itemType) {
    return itemType + QString::number(seqCounter++, 10);
}

//void
//Workspace::addInitialServersToModel() {
//    for (int i = 0; i < serverList.size(); i++) {
//        serverModel.appendServerEntry(serverList.get(i));
//    }
//}

//void
//Workspace::addInitialProjectsToModel() {
//    for (int i = 0; i < serverList.size(); i++) {
//        Server& server = serverList.get(i);

//        for (int k = 0; k < server.getProjectList().size(); k++) {
//            projectsModel.appendProjectEntry(server.getProjectList().get(k), server);
//        }
//    }
//}

//void
//Workspace::addInitialJobsToModel() {
//    for (int i = 0; i < jobList.size(); i++) {
//        jobModel.appendJobEntry(jobList.get(i));
//    }
//}

ServerListTableModel *
Workspace::getServerListTableModel() {
    return &serverModel;
}

//ProjectsListTableModel &
//Workspace::getProjectsListTableModel() {
//    return projectsModel;
//}

//JobListTableModel &
//Workspace::getJobListTableModel() {
//    return jobModel;
//}

void
Workspace::addServerToWorkSpace(Server &server) {
    serverModel.addServer(server);
    //serverList.addServer(server);
}

int
Workspace::serverCount() {
    return serverModel.servers.size();
    //return serverList.size();
}

Server
Workspace::getServer(int index) {
    return serverModel.servers.get(index);
    //return serverList.get(index);
}

//void
//Workspace::addProjectToWorkSpace(Project &project, Server *server) {
//    projectsModel.appendProjectEntry(project, *server);
//}

//void
//Workspace::addJobToWorkSpace(Job &job) {
//    jobModel.appendJobEntry(job);
//}

//void
//Workspace::startServerWatcher() {
//    serverWatcher.start();
//}

#endif
