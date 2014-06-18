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

#include <QHBoxLayout>
#include <QLabel>
#include "ServerTypePage.h"
#include <QMessageBox>
#include <QProgressBar>


#define REMOTE_SERVER_INDEX 1

ServerTypePage::ServerTypePage(QWidget *parent) : QWizardPage(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout();

    // Create the combo box to select the type of server
    serverTypeSelector = new QComboBox();
    serverTypeSelector->addItem("Local Server (localhost)");
    serverTypeSelector->addItem("Remote Server (access via SSH)");

    mainLayout->addWidget(new QLabel("Select a type of server to add:"));
    mainLayout->addWidget(serverTypeSelector, 0);

    buildRemoteServerWidget();
    remoteServerWidget->setEnabled(false);
    mainLayout->addWidget(remoteServerWidget, 0);

    setLayout(mainLayout);

    // Connect the combo box to the remote server widget so that
    // it enables or disables based on the type of server selected
    connect(serverTypeSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(serverTypeChanged(int)));

    // Register the serverName QLineEdit as a field so that its data
    // can be accessed later in the wizard.
    registerField("server", &serverName);
    registerField("userId", &userId);
    registerField("serverType", serverTypeSelector);
    // Set default value
    remoteConnectionVerified = false;
}



void
ServerTypePage::buildRemoteServerWidget() {
    remoteServerWidget = new QWidget();
    remoteServerLayout = new QVBoxLayout();
    // Add the title
    remoteServerLayout->addWidget(new QLabel("Remote Server Data"));
    // Create the horizontal layout for server info
    createServerInfoLayout();
    // Create the layout for the credentials
    createCredentialsLayout();
    remoteServerWidget->setLayout(remoteServerLayout);
}

void
ServerTypePage::createServerInfoLayout() {
    QHBoxLayout* serverInfoLayout = new QHBoxLayout();
    // Create the vertical layout for the server name
    QVBoxLayout* serverNameLayout = new QVBoxLayout();
    // Create label for server name field
    serverNameLayout->addWidget(new QLabel("Enter name or IP Address of server:"));
    // Set up server name field with default value
    serverName.setText("localhost");
    serverNameLayout->addWidget(&serverName);
    // Add serverName layout to its horizontal row
    serverInfoLayout->addLayout(serverNameLayout);
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
}

void
ServerTypePage::createCredentialsLayout() {
    // Create the credentials section
    QHBoxLayout* credentialsRow = new QHBoxLayout();
    // Username section
    QVBoxLayout* usernameLayout = new QVBoxLayout();
    usernameLayout->addWidget(new QLabel("Login (user) ID:"));
    userId.setText(getUserName());
    // Add the text field to its layout
    usernameLayout->addWidget(&userId);
    // Add the user id section to the credentials layout
    credentialsRow->addLayout(usernameLayout);
    // Password section
    QVBoxLayout* passwordLayout = new QVBoxLayout();
    // Add the password label
    passwordLayout->addWidget(new QLabel("Password:"));
    // Set the password field to use password characters
    password.setEchoMode(QLineEdit::Password);
    // Add the field to the layout
    passwordLayout->addWidget(&password);
    // Add the password layout to the credentials section
    credentialsRow->addLayout(passwordLayout);
    // Add the credentials section to the widget layout
    remoteServerLayout->addLayout(credentialsRow);
    // Set up the indicator for the remote connection tester.
    prgDialog.setLabelText("Please wait while we check the connection.");
    prgDialog.setCancelButton(0);
    prgDialog.setRange(0, 0);
    remoteServerLayout->addWidget(&prgDialog);
    prgDialog.setVisible(false);
}

void
ServerTypePage::serverTypeChanged(const int index) {

    remoteServerWidget->setEnabled(index == REMOTE_SERVER_INDEX);
    userId.setText( (index == REMOTE_SERVER_INDEX) ? "" : getUserName());
    serverName.setText( (index == REMOTE_SERVER_INDEX) ? "" : "localhost");

}

QString
ServerTypePage::getUserName() {
    // Gets the username on the system. "USER" for linux/Mac,
    // "USERNAME" for Windows
    QString userName = qgetenv("USER");

    return (!userName.isEmpty()) ? userName : qgetenv("USERNAME");
}

bool
ServerTypePage::validatePage() {

    if(serverTypeSelector->currentIndex() == REMOTE_SERVER_INDEX &&
            ! remoteConnectionVerified){
        prgDialog.setVisible(true);
        //Verify the server credentials.
        tester = new ServerConnectionTester(userId.text(), password.text(),
                                            serverName.text(), portNumber.value());
        // Allow us to know when tester has completed
        connect(tester, SIGNAL(finished()),
                this, SLOT(checkConnectionTesterResult()));

        //qRegisterMetaType<SshException>("SshException");

        // Test the remote connection
        tester->start();

        // Stay on the page for now while the test runs.
        return false;
    }
    // Hide the progress dialog
    prgDialog.setVisible(false);
    // If we tested a remote connection, delete the instance of it and
    // set it to false.
    if(remoteConnectionVerified) {
        delete tester;
        remoteConnectionVerified = false;
    }
    return true;
}

void
ServerTypePage::cleanupPage() {
    prgDialog.setVisible(false);
}

void
ServerTypePage::checkConnectionTesterResult() {
    remoteConnectionVerified = tester->getResult();

    remoteConnectionVerified ? wizard()->next() : prgDialog.setVisible(false);
}

#endif
