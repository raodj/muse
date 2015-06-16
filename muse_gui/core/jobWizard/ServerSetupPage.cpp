#ifndef SERVER_SETUP_PAGE_CPP
#define SERVER_SETUP_PAGE_CPP

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

#include "ServerSetupPage.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include "Workspace.h"

ServerSetupPage::ServerSetupPage() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    // Add the modifiers to the QComboBox
    memoryModifier.addItem("MB");
    memoryModifier.addItem("GB");
    // add the executable layout to the page's main layout
    makeServerWidget(mainLayout);
    // Register fields so data can be accessed later.
    registerField("server", &serverSelector);
    registerField("nodes", &computerNodes);
    registerField("cpusPerNode", &cpuPerNode);
    registerField("memoryPerNode", &memPerNode);
    registerField("estimatedRunTime", &runTime);
    registerField("memoryUnits", &memoryModifier, "currentText", "currentTextChanged()");
    registerField("wantsEmail", &receiveEmails);
    receiveEmails.setText("Receive email notifications");
    mainLayout->addWidget(&receiveEmails);
    setLayout(mainLayout);
    setTitle("Server Setup");
    // Allow spin box to go up to 1 trillion
    memPerNode.setMaximum(99999999);
}

void
ServerSetupPage::makeServerWidget(QVBoxLayout *layout) {
    QGroupBox* borderedWidget = new QGroupBox("Server Info");
    QVBoxLayout* serverWidgetLayout = new QVBoxLayout();
    // The first item in this widget is the server selection.
    serverWidgetLayout->addWidget(new QLabel("Select Server:"));
    serverWidgetLayout->addWidget(&serverSelector);
    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    // Add labels
    horizontalLayout->addWidget(new QLabel("Computer Nodes"));
    horizontalLayout->addWidget(new QLabel("CPUs per Node"));
    serverWidgetLayout->addLayout(horizontalLayout);
    // Reset the layout
    horizontalLayout = new QHBoxLayout();
    // Now add two spin boxes
    horizontalLayout->addWidget(&computerNodes);
    horizontalLayout->addWidget(&cpuPerNode);
    serverWidgetLayout->addLayout(horizontalLayout);
    // Reset the layout
    horizontalLayout = new QHBoxLayout();
    // Two more labels
    horizontalLayout->addWidget(new QLabel("Memory per Node"));
    horizontalLayout->addWidget(new QLabel("Data Size"));
    horizontalLayout->addWidget(new QLabel("Est. Run Time (Hrs)"));
    serverWidgetLayout->addLayout(horizontalLayout);
    // Reset the layout
    horizontalLayout = new QHBoxLayout();
    // Last two spin boxes
    horizontalLayout->addWidget(&memPerNode);
    horizontalLayout->addWidget(&memoryModifier);
    horizontalLayout->addWidget(&runTime);
    serverWidgetLayout->addLayout(horizontalLayout);
    // Apply SWL to be the main layout of the subwidget
    borderedWidget->setLayout(serverWidgetLayout);
    // Add the widget to the main page.
    layout->addWidget(borderedWidget);
    populateServerList();
}

void
ServerSetupPage::populateServerList() {
//    ServerList& serverList = Workspace::get()->getServerList();
//    for (int i = 0; i < serverList.size(); i++) {
//        serverSelector.addItem(serverList.get(i).getName());
//    }
}


#endif
