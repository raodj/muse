#ifndef SERVER_SELECTION_PAGE_CPP
#define SERVER_SELECTION_PAGE_CPP

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

#include "ServerSelectionPage.h"
#include "Core.h"
#include "Workspace.h"
#include <QVBoxLayout>
#include <QLabel>
#include "RemoteServerSession.h"
#include "LocalServerSession.h"

ServerSelectionPage::ServerSelectionPage() {
    ServerList& list = Workspace::get()->getServerList();
    for (int i = 0; i < list.size(); i++) {
        serverList.addItem(list.get(i).getName());
    }
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(new QLabel("Select the Server this project belongs to:"));
    layout->addWidget(&serverList);

    setLayout(layout);
    session = NULL;
}

bool
ServerSelectionPage::validatePage() {
    checkServerChosen(serverList.currentIndex());
    return true;
}

void
ServerSelectionPage::checkServerChosen(int index) {
    ServerList& list = Workspace::get()->getServerList();
    if (session != NULL) {
        delete session;
    }
    if (list.get(index).isRemote()) {
        session = new RemoteServerSession(list.get(index), NULL, "Creating Project");
    }
    else {
        session = new LocalServerSession(list.get(index), NULL, "Creating Project");
    }
    emit serverSelected(session);
}

#endif
