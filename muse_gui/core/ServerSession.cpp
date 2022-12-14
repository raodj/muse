#ifndef SERVER_SESSION_CPP
#define SERVER_SESSION_CPP

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

#include "ServerSession.h"
#include "Core.h"
#include "Workspace.h"

#include <QThread>

#include <functional>
#include <future>

const QString ServerSession::projectsDirName{ "projects" };
const QString ServerSession::jobsDirName{ "jobs" };
const QString ServerSession::scriptsDirName{ "scripts" };

ServerSession::ServerSession(Server server, QWidget* parent) :
     server{ std::move(server) }
{
}

void
ServerSession::manageServer(ChangeType change) {
    //if (thread.joinable()) {
    //    std::cout << "thread running" << std::endl;
   //     thread.join();
    //}

    switch (change) {
    case ChangeType::CONNECT:
        std::thread(std::bind(&ServerSession::connectToServer, this)).detach();
        break;
    case ChangeType::GET_OS_TYPE:
        std::thread(std::bind(&ServerSession::getOSType, this)).detach();
        //thread = std::thread(std::bind(&ServerSession::getOSType, this));
        break;
    case ChangeType::CREATE_DIR:
        //thread = std::thread(std::bind(&ServerSession::mkdir, this));
        break;
    case ChangeType::CREATE_SERVER:
        //thread = std::thread(std::bind(&ServerSession::createServerData, this));
        break;
    case ChangeType::DIR_EXISTS:
        //thread = std::thread(std::bind(&ServerSession::dirExists, this));
        break;
    case ChangeType::VALIDATE_SERVER:
        //thread = std::thread(std::bind(&ServerSession::validate, this));
        break;
    case ChangeType::SAVE_TO_WORKSPACE:
        //thread = std::thread(std::bind(&ServerSession::saveToWorkspace, this));
        break;
    default:
        // should never happen, but inform the user of the error just in case
        emit errorEncountered("Unknown change on server");
    }
}

void
ServerSession::saveToWorkspace() {
    Workspace* ws = Workspace::get();

    // Assign a unique id unless the server has one already.
    // This normally would return true.
    if (server.getID().isEmpty()) {
        server.setID(ws->reserveID("server"));
    }

    bool saved = true;

    ws->addServerToWorkSpace(server);
    if (ws->saveWorkspace() != "") {
        saved = false;
    }

    emit announceSaveToWorkspaceResult(saved);
}

#endif
