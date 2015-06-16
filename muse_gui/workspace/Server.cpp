#ifndef SERVER_CPP
#define SERVER_CPP

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

#include "Server.h"
#include "Workspace.h"

// Predefined constants consistent with XML schema values for the server status.
const QString Server::Installing = "installing";

// Predefined constants consistent with XML schema values for the server status.
const QString Server::InstallFailed = "install_failed";

// Predefined constants consistent with XML schema values for the server status.
const QString Server::Good = "good";

// Predefined constants consistent with XML schema values for the server status.
const QString Server::Uninstalling = "uninstalling";

// Predefined constants consistent with XML schema values for the server status.
const QString Server::UninstallFailed = "uninstall_failed";

// Predefined constants consistent with XML schema values for the server status.
const QString Server::ConnectFailed = "connect_failed";

// Predefined constants consistent with XML schema values for OS type
const QString Server::Linux = "linux";

// Predefined constants consistent with XML schema values for OS type
const QString Server::Unix = "unix";

// Predefined constants consistent with XML schema values for OS type
const QString Server::Windows = "windows";

// Predefined constants consistent with XML schema values for OS type
const QString Server::UnknownOS = "unknown_os";

Server::Server(QString pID, bool pRemote, QString pName, int pPort,
               QString pDescription, QString pUserID, QString pInstallPath,
               QString pOsType, QString pStatus) :
    ID(pID), remote(pRemote), name(pName), port(pPort),
    description(pDescription), userID(pUserID), installPath(pInstallPath),
    status(pStatus), osType(pOsType) {
    // Add the set of instance variables that must be serialized/deserialized.
//    addElement(XMLElementInfo("ID",          &ID));
//    addElement(XMLElementInfo("Remote",      &remote));
//    addElement(XMLElementInfo("Name",        &name));
//    addElement(XMLElementInfo("Port",        &port));
//    addElement(XMLElementInfo("Description", &description));
//    addElement(XMLElementInfo("UserID",      &userID));
//    addElement(XMLElementInfo("InstallPath", &installPath));
//    addElement(XMLElementInfo("Status",      &status));
//    addElement(XMLElementInfo("OSType",      &osType));
//    addElement(XMLElementInfo("ProjectList", &projects));
}

Server::Server(Json::Value value) {
    ID = QString::fromStdString(value["ID"].asString());
    remote = value["remote"].asBool();
    name = QString::fromStdString(value["name"].asString());
    port = value["port"].asInt();
    description = QString::fromStdString(value["description"].asString());
    userID = QString::fromStdString(value["userID"].asString());
    installPath = QString::fromStdString(value["installPath"].asString());
    status = QString::fromStdString(value["status"].asString());
    osType = QString::fromStdString(value["os"].asString());
}

bool
Server::operator==(const Server other) {
    return ID == other.getID() && remote == other.isRemote() && name == other.getName()
            && port == other.getPort() && description == other.getDescription()
            && userID == other.getUserID() && installPath == other.getInstallPath()
            && osType == other.getOS();
}

void
Server::setName(const QString &name) {
    this->name = name;
}

void
Server::setName(const std::string &name) {
    this->name = QString::fromStdString(name);
}

void
Server::setPassword(const QString& credential) {
    password = credential;
}

void
Server::setDescription(const QString& description) {
    this->description = description;
}

void
Server::setOS(const QString& os) {
    osType = os;
}

void
Server::setInstallPath(const QString &path) {
    installPath = path;
}

void
Server::setID(const QString &id) {
    this->ID = id;
}

void
Server::addProject(Project& project) {
    projects.push_back(project);
//    projects.addProject(project);
//    Workspace::get()->addProjectToWorkSpace(project, this);
}

void
Server::addJob(Job &job) {
    jobs.push_back(job);
}

void
Server::update() {

}

std::vector<Project>
Server::getProjects() {
    return projects;
}

std::vector<Job>
Server::getJobs() {
    return jobs;
}

Json::Value
Server::save() {
    Json::Value item;

    item["ID"] = ID.toStdString();
    item["remote"] = remote;
    item["name"] = name.toStdString();
    item["port"] = port;
    item["description"] = description.toStdString();
    item["useID"] = userID.toStdString();
    item["installPath"] = installPath.toStdString();
    item["status"] = status.toStdString();
    item["os"] = osType.toStdString();

    return item;
}

#endif
