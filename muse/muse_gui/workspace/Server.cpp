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
//   /\  \    abide  by the intellectual  property laws,  and all other225
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
const QString Server::OSX = "darwin";

// Predefined constants consistent with XML schema values for OS type
const QString Server::UnknownOS = "unknown_os";

Server::Server(QString pID, bool pRemote, QString pName, int pPort,
               QString pDescription, QString pUserID, QString pInstallPath,
               QString pOsType, QString pStatus) : XMLElement("Server"),
    ID(pID), remote(pRemote), name(pName), port(pPort),
    description(pDescription), userID(pUserID), installPath(pInstallPath),
    status(pStatus), osType(pOsType) {
    // Add the set of instance variables that must be serialized/deserialized.
    addElement(XMLElementInfo("ID",          &ID));
    addElement(XMLElementInfo("Remote",      &remote));
    addElement(XMLElementInfo("Name",        &name));
    addElement(XMLElementInfo("Port",        &port));
    addElement(XMLElementInfo("Description", &description));
    addElement(XMLElementInfo("UserID",      &userID));
    addElement(XMLElementInfo("InstallPath", &installPath));
    addElement(XMLElementInfo("Status",      &status));
    addElement(XMLElementInfo("OSType",      &osType));
    //addElement(XMLElementInfo("ProjectList", &projects));
}

Server::Server(const Server &server) : XMLElement("Server"),
    ID(server.ID), remote(server.remote), name(server.name),
    port(server.port), description(server.description),
    userID(server.userID), installPath(server.installPath),
    status(server.status), osType(server.osType) {
    addElement(XMLElementInfo("ID",          &ID));
    addElement(XMLElementInfo("Remote",      &remote));
    addElement(XMLElementInfo("Name",        &name));
    addElement(XMLElementInfo("Port",        &port));
    addElement(XMLElementInfo("Description", &description));
    addElement(XMLElementInfo("UserID",      &userID));
    addElement(XMLElementInfo("InstallPath", &installPath));
    addElement(XMLElementInfo("Status",      &status));
    addElement(XMLElementInfo("OSType",      &osType));
}

bool
Server::operator==(const Server& other) {
    return ID == other.getID() && remote == other.isRemote() && name == other.getName()
            && port == other.getPort() && description == other.getDescription()
            && userID == other.getUserID() && installPath == other.getInstallPath()
            && osType == other.getOS();
}

void
Server::setName(const QString &name) {
    this->name = name;
    emit serverUpdated(*this);
}

void
Server::setName(const std::string &name) {
    this->name = QString::fromStdString(name);
    emit serverUpdated(*this);
}

void
Server::setPassword(const QString& credential) {
    password = credential;
    emit serverUpdated(*this);
}

void
Server::setDescription(const QString& description) {
    this->description = description;
    emit serverUpdated(*this);
}

void
Server::setOS(const QString& os) {
    osType = os;
    emit serverUpdated(*this);
}

void
Server::setInstallPath(const QString &path) {
    installPath = path;
    emit serverUpdated(*this);
}

void
Server::setID(const QString &id) {
    this->ID = id;
    emit serverUpdated(*this);
}

//void
//Server::addProject(Project& project) {
//    projects.addProject(project);
//    emit serverUpdated(*this);
//}

QString
Server::getHomeDir() const {
    if (osType == Linux || osType == Unix) {
        return "/home/";
    }

    if (osType == OSX) {
        return "/Users/";
    }

    if (osType == Windows) {
        return "C:\\Users\\";
    }

    return "";
}

QString
Server::separator() const {
    if (osType == Windows) {
        return "\\";
    }

    return "/";
}

bool
Server::isSameServer(const Server& other) {
    return remote == other.isRemote() && name == other.getName()
            && port == other.getPort() && description == other.getDescription()
            && userID == other.getUserID() && installPath == other.getInstallPath()
            && status == other.getStatus() && osType == other.getOS();
}

#endif
