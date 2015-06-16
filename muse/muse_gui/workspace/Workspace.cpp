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
#include "json/jsonHelper.h"
#include "Server.h"

#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

#include <jsoncpp/json/json.h>

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace muse {
namespace workspace {

/*
 * private data stored about the workspace is kept in an anonymous namespace
 * so it is impossible to access from outside Workspace.cpp
 */
namespace {

QDateTime timestamp;

std::vector<Server> servers;

std::thread serverWatcher;

long int counter;

const QString saveDataFileName = "servers.json";
const QString knownHostsFileName = "known_hosts";

bool checkForNeededFile() {
    // first, create the known hosts file if it doesnt exist, this is needed
    // whether or not we are creating a new workspace or using an existing one
    QFile knownHostsFile(knownHostsFilePath());

    if (!knownHostsFile.exists() && !knownHostsFile.open(QFile::WriteOnly)) {
        throw QString("Failed to create known hosts file");
    }

    QFileInfo file(saveDataFilePath());

    // the existance of this file will determine if we create a new workspace
    // or use what is already there
    return file.exists() && file.isFile() && file.isReadable();
}

void useExisting() {
    if (!muse::json::valid(saveDataFilePath())) {
        throw QString("Failed to load workspace save data");
    }

    // the json file is valid, now load our workspace
    load();
}

void createNew() {
    // create a json file to save our workspace data to
    QFile saveDataFile(saveDataFilePath());

    if (!saveDataFile.exists() && !saveDataFile.open(QFile::WriteOnly)) {
        throw QString("Failed to create save data file");
    }

    // now that the json file has been created, save what we currently have to it
    save();
}

void serverWatcherExec() {
    while (true) {
        std::cout << "server watcher running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

} // end anonymous namespace


/*
 * functions visible to the rest of muse gui
 */

void init() {
    timestamp = QDateTime::currentDateTime();
    counter = 0;

    if (checkForNeededFile()) {
        useExisting();
    } else {
        createNew();
    }

    serverWatcher = std::thread(serverWatcherExec);
}

void save() {
    Json::Value root = Json::objectValue;
    Json::Value list = root["server-list"];
    list = Json::arrayValue;

    timestamp = QDateTime::currentDateTime();

    root["directory"] = appDir().toStdString();
    root["timestamp"] = timestamp.toString().toStdString();
    root["counter"] = (int) counter;

    for (auto& s : servers) {
        list.append(s.save());
    }

    muse::json::save(saveDataFilePath(), root);
}

void load() {
    Json::Value root = muse::json::load(saveDataFilePath());
    Json::Value list = root["server-list"];

    timestamp = QDateTime::fromString(QString::fromStdString(root["timestamp"].asString()));
    counter = root["counter"].asInt();

    for (auto& item : list) {
        addServer(Server(item));
    }

    muse::json::save(saveDataFilePath(), root);
}

void clear() {
    servers.clear();
}

void addServer(Server server) {
    servers.push_back(server);
}

void removeServer(Server server) {
    servers.erase(std::remove(std::begin(servers), std::end(servers), server), std::end(servers));
}

bool firstRun() {
    return !checkForNeededFile();
}

std::vector<Server> getServers() {
    return servers;
}

int serverCount() {
    return servers.size();
}

Server getServer(int index) {
    return servers.at(index);
}

QString appDir() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString knownHostsFilePath() {
    return appDir() + QDir::separator() + knownHostsFileName;
}

QString saveDataFilePath() {
    return appDir() + QDir::separator() + saveDataFileName;
}

QString reserveID(const QString &itemType) {
    return itemType + QString::number(counter++);
}

} // end namespace workspace
} // end namespace muse
/*
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
    attributes.append("Version", "0.1");

    // Add the top-level elements to the base class.
    addElement(XMLElementInfo("Directory",  &directory));
    addElement(XMLElementInfo("Timestamp",  &timestamp));
    addElement(XMLElementInfo("SeqCounter", &seqCounter));
    //addElement(XMLElementInfo("ServerList", &serverList));
    //addElement(XMLElementInfo("JobList", &jobList));
}

void
Workspace::registerClasses() {
    // Register all the underlying sub-elements for unmarshalling
    qRegisterMetaType<ServerList>("ServerList");
    qRegisterMetaType<Server>("Server");
    //qRegisterMetaType<ProjectList>("ProjectList");
    //qRegisterMetaType<Project>("Project");
    //qRegisterMetaType<JobList>("JobList");
    //qRegisterMetaType<Job>("Job");
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

    //if (muse::json::validateFile(wsDir + "servers.json")) {
    //    std::cout << "json was valid" << std::endl;
    //

    //workspace->serverList = muse::json::getServerList(wsDir + "server.json");

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
    //XMLParser xmlParser;
    QDir dir(directory);
    const QString wsDir = dir.absolutePath() + QDir::separator();
    Workspace *ws = new Workspace();
    QString errMsg;
    //QString schemaFile = ":/resources/muse_gui.xsd";

    //if ((errMsg = xmlParser.loadXML(wsFile.filePath(),
    //                                schemaFile, *ws)) != "") {
        // Error occurred. Do no further operations.
    //    delete ws;
    //    return errMsg;
    //}

    if (!muse::json::validateFile(wsDir + "servers.json")) {
        std::cout << "json was invalied" << std::endl;
        delete ws;
        return errMsg;
    }

    ws->serverList = muse::json::getServerList(wsDir + "servers.json");

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
    workspace->addInitialServersToModel();
    workspace->addInitialProjectsToModel();
    workspace->addInitialJobsToModel();

    // Start the thread that will continuously monitor the servers for updates
    workspace->startServerWatcher();

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

void
Workspace::addInitialProjectsToModel() {
    for (int i = 0; i < serverList.size(); i++) {
        Server& server = serverList.get(i);

        for (int k = 0; k < server.getProjectList().size(); k++) {
            projectsModel.appendProjectEntry(server.getProjectList().get(k), server);
        }
    }
}

void
Workspace::addInitialJobsToModel() {
    for (int i = 0; i < jobList.size(); i++) {
        jobModel.appendJobEntry(jobList.get(i));
    }
}

ServerListTableModel &
Workspace::getTableModel() {
    return serverModel;
}

ProjectsListTableModel &
Workspace::getProjectsListTableModel() {
    return projectsModel;
}

JobListTableModel &
Workspace::getJobListTableModel() {
    return jobModel;
}

void
Workspace::addServerToWorkSpace(Server &server) {
    serverList.addServer(server);
    serverModel.appendServerEntry(server);
}

void
Workspace::addProjectToWorkSpace(Project &project, Server *server) {
    projectsModel.appendProjectEntry(project, *server);
}

void
Workspace::addJobToWorkSpace(Job &job) {
    jobModel.appendJobEntry(job);
}

void
Workspace::startServerWatcher() {
    serverWatcher.start();
}
*/

#endif
