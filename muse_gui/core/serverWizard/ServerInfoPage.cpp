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
    installDirectoryVerified = mkdirSucceeded = false;
    serverSession = NULL;
    setTitle("Server Data");
    setSubTitle("Additional Information");
}

void
ServerInfoPage::initializePage() {
    // Only enable the button if the user is creating a local server.
    browse.setEnabled((field("serverType") == LOCAL_SERVER));
    // Set the default text for the install directory
    installDirectoryDisplay.setText( (field("serverType") == LOCAL_SERVER) ? "" :
                                  "/home/" + field("userId").toString() + "/MUSE");

}

bool
ServerInfoPage::validatePage() {
    prgDialog.setVisible(true);
    // Step 1: mkdir
    if (!mkdirSucceeded) {
        // connect the signal to learn of the result.
        connect(serverSession, SIGNAL(directoryCreated(bool)),
                this, SLOT(getMkdirResult(bool)));
        // Attempt to make the directory
        serverSession->mkdir(installDirectoryDisplay.text());
        // Stay on the page
        return false;
    }
    // Step 2: rmdir
   else if (mkdirSucceeded && !installDirectoryVerified) {
        // connect the signal to learn of the result
        connect(serverSession, SIGNAL(directoryRemoved(bool)),
                this, SLOT(getRmdirResult(bool)));
        // Attempt to remove the directory
        serverSession->rmdir(installDirectoryDisplay.text());
        // Stay on the page.
        return false;
    }
    // Step 3: Advance to next page
    else {
        // Disconnect the signals
        disconnect(serverSession, SIGNAL(directoryCreated(bool)),
                   this, SLOT(getMkdirResult(bool)));
        disconnect(serverSession, SIGNAL(directoryRemoved(bool)),
                   this, SLOT(getRmdirResult(bool)));
        // Ensure that a return to this page reverifies everything
        mkdirSucceeded = false;
        installDirectoryVerified = false;
        Server *server = serverSession->getServer();
        // Add the server description.
        server->setDescription((serverDescription.toPlainText().isEmpty()) ?
                                  "None provided." : serverDescription.toPlainText());
        // Add the install directory
        server->setInstallPath(installDirectoryDisplay.text());
        // Inform the user of the success.
        QMessageBox msgBox;
        msgBox.setText(ServerInfoPage::InstallDirectoryMessage);
        msgBox.setDetailedText("More info to come later...");
        msgBox.setWindowTitle("Install Validation Success");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        // Finally, we can advance to the next page.
        return true;
    }
}

void
ServerInfoPage::setServerSessionPointer(ServerSession *ss) {
    //remoteServerSession = rss;
    serverSession = ss;
}

void
ServerInfoPage::getMkdirResult(bool result) {
    mkdirSucceeded = result;
    if (!mkdirSucceeded) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("There was an issue creating the install directory.<br/>"\
                       "This likely means that the directory currently exists," \
                       " which is not good. Please change the install directory.");
        msgBox.exec();
        // Hide the dialog since we are stopping the process.
        prgDialog.setVisible(false);
    }
    else {
        // mkdirSucceeded is true, we can move on to the next step
        wizard()->next();
    }
}

void
ServerInfoPage::getRmdirResult(bool result) {
    installDirectoryVerified = result;
    if (!installDirectoryVerified) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("There was an issue deleting the install directory.<br/>"\
                       "This is an unusual occurrence, and likely means that the," \
                       " connection to the server was dropped.");
        msgBox.exec();
        // Couldn't remove the directory, return to initial state
        mkdirSucceeded = false;
        prgDialog.setVisible(false);
    }
    else {
        // We removed the directory, we can move on.
        wizard()->next();
    }
}

void
ServerInfoPage::browseFileSystem() {
    QFileDialog fileDiag;
    QString dirPath = fileDiag.getExistingDirectory(NULL,
                                                    "TYPE the name of the directory to CREATE",
                                  QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    if (!dirPath.isEmpty()) {
        installDirectoryDisplay.setText(dirPath);
        QDir dir;
        // The user thinks they created the directory.
        // It's irrelevant if rmdir fails or not.
        dir.rmdir(dirPath);
    }

}

#endif