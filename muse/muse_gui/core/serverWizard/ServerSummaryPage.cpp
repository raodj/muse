#ifndef SERVER_SUMMARY_PAGE_CPP
#define SERVER_SUMMARY_PAGE_CPP

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

#include "ServerSummaryPage.h"
#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QVBoxLayout>
#include "Workspace.h"
#include "ServerList.h"
#include <QMessageBox>

ServerSummaryPage::ServerSummaryPage(QWidget* parent) : QWizardPage(parent) {

    // Set up the layout
    QVBoxLayout* mainLayout = new QVBoxLayout();
    // User can't edit the text in this view port.
    summaryText.setReadOnly(true);

    // load the text from the html file
    QFile page(":/resources/serverSummary.html");
    if(page.open(QFile::ReadOnly)){
        QTextStream input(&page);

        summaryText.setHtml(input.readAll());
        page.close();
    }
    // Add the text to the layout
    mainLayout->addWidget(&summaryText);
    // Add label for server name
    mainLayout->addWidget(new QLabel("Server's host name:"));
    // Place the QLineEdit containing the server name
    mainLayout->addWidget(&serverName);
    // Add label for install directory
    mainLayout->addWidget(new QLabel("Install directory:"));
    // Place QLineEdit contianing install directory
    mainLayout->addWidget(&installDirectory);
    // Set mainLayout as default layout
    setLayout(mainLayout);
    serverSession = NULL;
}

void
ServerSummaryPage::initializePage() {
    serverName.setText(field("server").toString());
    installDirectory.setText(field("installPath").toString());
}

bool
ServerSummaryPage::validatePage() {
    Workspace* ws = Workspace::get();
    // Get the server.
    Server* server = serverSession->getServer();
    // Assign a unique id unless the server has one already.
    // This normally would return true.
    if (server->getID().isEmpty()) {
        server->setID(ws->reserveID("server"));
    }

    // Add the server to the workspace
    ws->addServerToWorkSpace(*server);
    QString err;
    err = ws->saveWorkspace();
    // If err has text, we had a problem.
    if (err != "") {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error saving server to workspace.");
        msgBox.setText(err);
        return false;
    }

    // For now, delete the session to avoid memory leaks.
    // When installation gets implemented, this line should be
    // removed.
    delete serverSession;
    // Saved successfully, close the wizard.
    return true;
}

void
ServerSummaryPage::setServerSessionPointer(ServerSession *ss) {
    serverSession = ss;
}

#endif
