#ifndef SERVER_LIST_CPP
#define SERVER_LIST_CPP


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

#include "ServerList.h"
#include <QDebug>

#include <iostream>

ServerList::ServerList() : XMLElement("ServerList") {
    // Register elements in the order in which they shlould occur.
    qRegisterMetaType<XMLElement>("XMLElement");
    addElement(XMLElementInfo("Server", &servers));
}

//ServerList::ServerList(const ServerList &list) : XMLElement("ServerList") {
//    servers = list.servers;
//   // addElement(XMLElementInfo("Server", &servers));
//}

void
ServerList::addServer(const Server &entry) {
   Server* server = new Server(entry.getID(), entry.isRemote(), entry.getName(), entry.getPort(),
                               entry.getDescription(), entry.getUserID(), entry.getInstallPath(),
                               entry.getOS(), entry.getStatus());
   servers.append(server);
   //addElement(XMLElementInfo("Server", server));
   QObject::connect(server, SIGNAL(serverUpdated(Server)),
                    this, SLOT(serverUpdated(Server)));
   // Fire signal indicating change to the server list.
   const int index = servers.size() - 1;
   emit serverChanged(ENTRY_INSERTED, index, index);
}

void
ServerList::clear() {
    for (int i = servers.size(); i >= 0; i--) {
        emit serverChanged(ENTRY_DELETED, i, i);
    }

    servers.clear();
}

int
ServerList::getIndex(const QString& serverID) const {
    for (int i = 0; (i < servers.size()); i++) {
        if (get(i).getID() == serverID) {
            return i;
        }
    }
    return -1;
}

void
ServerList::serverUpdated(const Server &server) {
    const int index = getIndex(server.getID());
    if (index != -1) {
        emit serverChanged(ENTRY_UPDATED, index, index);
    }
}

#endif
