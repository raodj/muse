#ifndef SERVER_TYPE_PAGE_CPP
#define SERVER_TYPE_PAGE_CPP

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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "ServerTypePage.h"

ServerTypePage::ServerTypePage(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout();

    serverTypeSelector = new QComboBox();
    serverTypeSelector->addItem("Local Server (localhost)");
    serverTypeSelector->addItem("Remote Server (access via SSH)");

    mainLayout->addWidget(new QLabel("Select a type of server to add:"));
    mainLayout->addWidget(serverTypeSelector, 0);

    buildRemoteServerWidget();
    mainLayout->addWidget(remoteServerWidget, 1);

    setLayout(mainLayout);


}

void
ServerTypePage::buildRemoteServerWidget() {
    remoteServerWidget = new QWidget();
    QVBoxLayout* remoteServerLayout = new QVBoxLayout();
    // Add the title
    remoteServerLayout->addWidget(new QLabel("Remote Server Data"));
    // Create the horizontal layout for server info
    QHBoxLayout* serverInfoLayout = new QHBoxLayout();
    // Create the vertical layout for the server name
    QVBoxLayout* serverNameLayout = new QVBoxLayout();
    // Create label for server name field
    serverNameLayout->addWidget(new QLabel("Enter name or IP Address of server:"));
    // Set up server name field with default value
    serverName.setText("localhost");
    serverNameLayout->addWidget(&serverName);
    // Add serverName layout to its horizontal row
    serverInfoLayout->addLayout(serverNameLayout, 0);
    // Vertical layout for port number
    QVBoxLayout* portLayout = new QVBoxLayout();
    // Add Port label
    portLayout->addWidget(new QLabel("Port:"));
    // Set up port number
    portNumber.setValue(22);
    portLayout->addWidget(&portNumber);
    // Add port layout to its row
    serverInfoLayout->addLayout(portLayout);
    // Add server info layout to widget layout
    remoteServerLayout->addLayout(serverInfoLayout);



    remoteServerWidget->setLayout(remoteServerLayout);
}

void
ServerTypePage::serverTypeChanged() {

}

#endif
