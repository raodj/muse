#ifndef SERVER_INFO_PAGE_CPP
#define SERVER_INFO_PAGE_CPP

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

#define LOCAL_SERVER "0"
#include "ServerInfoPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

const QString ServerInfoPage::InstallDirectoryMessage =
        "The install directory does not exist (this is good), and"\
        " will be created when MUSE runtime is installed.";

ServerInfoPage::ServerInfoPage(QWidget *parent) : QWizardPage(parent) {
    // Set up the main layout
    QVBoxLayout* mainLayout = new QVBoxLayout();

    // Add label for server description
    mainLayout->addWidget(new QLabel("Description of server (for your reference):"));

    // Set the QTextEdit to be editable
    serverDescription.setReadOnly(false);

    // Add the QTextEdit to the layout
    mainLayout->addWidget(&serverDescription);

    // Add label for install path
    mainLayout->addWidget(new QLabel("Enter install directory (absolute path)"));

    //Create horizontal layout for install directory and browse button
    QHBoxLayout* directoryLayout = new QHBoxLayout();

    // Add line edit to layout
    directoryLayout->addWidget(&installDirectoryDisplay);

    // if the user changes the selected directory, we need to recheck if it
    // exists
    connect(&installDirectoryDisplay, &QLineEdit::textChanged,
            [&] (const QString& text) {
        Q_UNUSED(text);
        installDirChecked = false;
        installDirExists = false;
        installDirValidated = false;
        installDirHasServerData = false;
    });

    // Set up browse button
    browse.setText("Browse");

    // Add the button to the directory layout
    directoryLayout->addWidget(&browse);

    // Allow the button to trigger the QFileDialog
    connect(&browse, SIGNAL(clicked()), this, SLOT(browseFileSystem()));

    // Add the directory layout to the overall layout
    mainLayout->addLayout(directoryLayout);

    // Set the spinbox to a default first value
    pollingDelay.setValue(30);

    // Add the label for the polling delay
    mainLayout->addWidget(new QLabel("Enter polling delay (seconds):"));
    mainLayout->addWidget(&pollingDelay);

    // Set up the indicator for the remote connection tester.
    prgDialog.setLabelText("Please wait while we verify the install directory...");
    prgDialog.setCancelButton(0);
    prgDialog.setRange(0, 0);
    prgDialog.setVisible(false);
    mainLayout->addWidget(&prgDialog);

    // Register the install directory as a field
    registerField("installPath*", &installDirectoryDisplay);
    setLayout(mainLayout);

    // Default value
    installDirectoryVerified = false;
    mkdirSucceeded = false;

    installDirChecked = false;
    installDirExists = false;
    installDirValidated = false;
    installDirHasServerData = false;

    serverSession = nullptr;

    setTitle("Server Data");
    setSubTitle("Additional Information");
}

void
ServerInfoPage::initializePage() {
    // Only enable the button if the user is creating a local server.
    browse.setEnabled((field("serverType") == LOCAL_SERVER));

    if (field("serverType") == LOCAL_SERVER) {
        installDirectoryDisplay.setText("");
    } else {
        installDirectoryDisplay.setText(serverSession->getServer()->getHomeDir() +
                                        field("userId").toString() + serverSession->getServer()->separator() +
                                        "MUSE");
    }
}

bool
ServerInfoPage::validatePage() {
    prgDialog.setVisible(true);

    // The first thing to do is check to make sure the install directory exists
    if (!installDirChecked) {
        connect(serverSession, SIGNAL(directoryExists(bool)),
                this, SLOT(getDirExistsResult(bool)));

        serverSession->dirExists(installDirectoryDisplay.text());

        return false;
    }

    // Next, if the directory doesnt exist, we nee to create it
    if (!installDirExists) {
        connect(serverSession, SIGNAL(directoryCreated(bool)),
                this, SLOT(getMkdirResult(bool)));

        serverSession->mkdir(installDirectoryDisplay.text());

        return false;
    }

    // The directory exists (it may have just been created) but now we
    // need to add the necessary files and directories to make this a valid
    // Server
    if (!installDirHasServerData) {
        connect(serverSession, SIGNAL(serverDataCreated(bool)),
                this, SLOT(getServerDataCreatedResult(bool)));

        serverSession->createServerData(installDirectoryDisplay.text());

        return false;
    }

    // The install directory the user chose already exists, so if we have
    // not yet done so, we need to check if its a valid server
    if (!installDirValidated) {
        connect(serverSession, SIGNAL(directoryValidated(bool)),
                this, SLOT(getDirValidatedResult(bool)));

        serverSession->validate(installDirectoryDisplay.text());

        return false;
    }

    // At this point, we are ready to go to the next page in the wizard
    disconnect(serverSession, SIGNAL(directoryCreated(bool)),
               this, SLOT(getMkdirResult(bool)));
    disconnect(serverSession, SIGNAL(directoryExists(bool)),
               this, SLOT(getDirExistsResult(bool)));
    disconnect(serverSession, SIGNAL(serverDataCreated(bool)),
               this, SLOT(getServerDataCreatedResult(bool)));
    disconnect(serverSession, SIGNAL(directoryValidated(bool)),
               this, SLOT(getDirValidatedResult(bool)));

    // Ensure that a return to this page reverifies everything
    installDirChecked = false;
    installDirExists = false;
    installDirValidated = false;

    Server *server = serverSession->getServer();

    // Add the server description.
    server->setDescription((serverDescription.toPlainText().isEmpty()) ?
                              "None provided." : serverDescription.toPlainText());

    // Add the install directory
    server->setInstallPath(installDirectoryDisplay.text());

    // Finally, we can advance to the next page.
    return true;
}

void
ServerInfoPage::setServerSessionPointer(ServerSession *ss) {
    serverSession = ss;
}

void
ServerInfoPage::getMkdirResult(bool result) {
    // If mkdir() failed then we need to check the existance of the dir next
    // time we try to advance the wizard
    installDirChecked = result;
    installDirExists = result;

    if (!installDirExists) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("There was an issue creating the install directory.<br/>"\
                       "This likely means that the directory currently exists," \
                       " which is not good. Please change the install directory.");
        msgBox.exec();

        // Hide the dialog since we are stopping the process.
        prgDialog.setVisible(false);
    } else {
        wizard()->next();
    }
}

void
ServerInfoPage::getDirExistsResult(bool result) {
    // We dont need to check the existance of this directory again, regardless
    // of the result of dirExists()
    installDirChecked = true;
    installDirExists = result;

    // We will assume for now that if the user is selecting an existing directory
    // then it already has server data, but we need to make sure it is valid
    installDirHasServerData = result;
    installDirValidated = false;

    QMessageBox message;

    if (installDirExists) {
        message.setWindowTitle("Directory Exists");
        message.setText("The directory you have chosen already exists, so we "\
                        "will attempt to load the data from this server if it "\
                        "is valid.");
        message.exec();
    } else {
        message.setWindowTitle("Directory Does Not Exist");
        message.setText("The directory you have chosen does not exist, so we "\
                        "will attempt to create it and setup all the necessary "\
                        "data needed for the server");
        message.exec();
    }

    wizard()->next();
}

void
ServerInfoPage::getServerDataCreatedResult(bool result) {
    // If the creating the server data was a success then we are done checking
    // this directory, if not, we need to recheck it completely the next time
    installDirChecked = result;
    installDirExists = result;
    installDirHasServerData = result;
    installDirectoryVerified = result;

    // If the server data was successfully created, we can advance in the
    // wizard, otherwise we need to inform the user that there was a problem
    if (installDirHasServerData) {
        wizard()->next();
    } else {
        QMessageBox message;
        message.setWindowTitle("Error creating necessary server files");
        message.setText("There was an error creating the required server files.<br/>"\
                        "This is an unusual occurrence, and likely means that the, "\
                        "connection to the server was dropped.");
        message.exec();
    }
}

void
ServerInfoPage::getDirValidatedResult(bool result) {
    // If the dir is a valid Server, then we dont need to validate it again or
    // create any new Server data
    installDirValidated = result;
    installDirHasServerData = result;

    // If it turns out this is not a valid server, we need to ask the user
    // what they want to do about it
    if (!installDirValidated) {
        auto reply = QMessageBox::question(this, "This directory is not a valid server",
                                           "The directory you have chosen does not contain "\
                                           "any valid server information, do you wish to create "\
                                           "a new Server in this directory?",
                                           QMessageBox::Yes | QMessageBox::No);

        // If the user does not want to use this directory as a new Server then
        // make sure we completely recheck the directory next time we try to
        // advance in the wizard
        if (reply == QMessageBox::No) {
            installDirChecked = false;
            installDirExists = false;
            installDirHasServerData = false;
            installDirValidated = false;

            return;
        }
    }

    wizard()->next();
}

void
ServerInfoPage::browseFileSystem() {
    QFileDialog fileDiag;
    QString dirPath = fileDiag.getExistingDirectory(nullptr,
                                                    "TYPE the name of the directory to CREATE",
                                                    QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
}

#endif
