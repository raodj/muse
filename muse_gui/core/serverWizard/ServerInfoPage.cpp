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
    registerField("installPath", &installDirectoryDisplay);
    setLayout(mainLayout);
    // Default value
    installDirectoryVerified = mkdirSucceeded = false;
}

void
ServerInfoPage::initializePage() {
    // Only enable the button if the user is creating a local server.
    browse.setEnabled((field("serverType") == LOCAL_SERVER));
    // Set the default text for the install directory
    installDirectoryDisplay.setText("/home/" + field("userId").toString() + "/MUSE");

}

bool
ServerInfoPage::validatePage() {

    if( (field("serverType")) != LOCAL_SERVER && !installDirectoryVerified){
        prgDialog.setVisible(true);
        // Connect signal to know success of mkdir
        connect(remoteServerSession, SIGNAL(booleanResult(bool)),
                this, SLOT(getMkdirResult(bool)));
        // Verify the install path by making the directory (if it doesn't exist)
        remoteServerSession->mkdir(installDirectoryDisplay.text());
        // Stay on this page
        return false;
    }
    else {
        // Nothing for now until LocalSS is implemented

    }
    prgDialog.setVisible(false);
    if (installDirectoryVerified && (field("serverType")) != LOCAL_SERVER) {
        // Returns to this page must verify again
        installDirectoryVerified = false;
        mkdirSucceeded = false;
        // if server is remote, disconnect the signal to getRmdirResult
        if( (field("serverType")) != LOCAL_SERVER) {
            disconnect(remoteServerSession, SIGNAL(booleanResult(bool)),
                       this, SLOT(getRmdirResult(bool)));

            // POSSIBLY ADD POLLING DELAY.------------------------------------------
        }

            return true;
    }
    // installDirectoryVerified was false...
    mkdirSucceeded = false;

    return false;
}

void
ServerInfoPage::setServerSessionPointer(RemoteServerSession *rss) {
    remoteServerSession = rss;
}

void
ServerInfoPage::getRmdirResult(bool result) {
    installDirectoryVerified = result && mkdirSucceeded;
   // If everything is good, move on.
    if (installDirectoryVerified) {
        QMessageBox msgBox;
        msgBox.setText(ServerInfoPage::InstallDirectoryMessage);
        msgBox.setDetailedText("More info to come later...");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        // Apply the description of the server to the workspace.
        remoteServerSession->getServer()->
                setDescription(serverDescription.toPlainText());
        // Apply the install path of the server to the workspace.
        remoteServerSession->getServer()->
                setInstallPath(installDirectoryDisplay.text());

        wizard()->next();
    }
    else {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("There was an issue deleting the install directory.<br/>"\
                       "This is an unusual occurrence, and likely means that the," \
                       " connection to the server was dropped.");
        msgBox.exec();
        // Couldn't remove the directory, return to initial state
        mkdirSucceeded = false;
    }
}

void
ServerInfoPage::getMkdirResult(bool result) {
    mkdirSucceeded = result;
    if (!result) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Error");
        msgBox.setText("There was an issue creating the install directory.<br/>"\
                       "This likely means that the directory currently exists," \
                       " which is not good. Please change the install directory.");
        msgBox.exec();
        prgDialog.setVisible(false);
    }
    else {
        // We successfully made the directory, so now, we want to
        // connect the signal to the getRmdDirResult slot, so
        // let's disconnect the getMkDirResult.
        disconnect(remoteServerSession, SIGNAL(booleanResult(bool)),
                   this, SLOT(getMkdirResult(bool)));
        // Connect the signal to getRmdirResult
        connect(remoteServerSession, SIGNAL(booleanResult(bool)),
                this, SLOT(getRmdirResult(bool)));
        // We passed the first step, now remove it.
        remoteServerSession->rmdir(installDirectoryDisplay.text());
    }
}

#endif
