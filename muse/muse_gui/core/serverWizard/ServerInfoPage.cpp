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

ServerInfoPage::ServerInfoPage(QWidget *parent) : QWizardPage(parent) {

    // Set up the main layout
    QVBoxLayout* mainLayout = new QVBoxLayout();
    // Add label for server description
    mainLayout->addWidget(new QLabel("Description for server (for your reference):"));
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



    // Register the install directory as a field
    registerField("installPath", &installDirectoryDisplay);


    setLayout(mainLayout);
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
    //Verify the install path. For now, return true
    return true;
}

#endif
