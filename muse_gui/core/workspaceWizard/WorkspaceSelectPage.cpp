#ifndef WORKSPACE_SELECT_PAGE_CPP
#define WORKSPACE_SELECT_PAGE_CPP

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

#include "WorkspaceSelectPage.h"
#include "Workspace.h"
#include "MUSEGUIApplication.h"

#include <QLabel>
#include <QFileDialog>
#include <QDialog>
#include <QStringList>
#include <QMessageBox>
#include <QFileInfo>

#include <iostream>

const QString WorkspaceSelectPage::workspaceExistsMessage =
        "The workspace you have selected already exists, would you like to " \
        "continue creating a new workspace in this directory?  All data from " \
        "the existing workspace will be lost.";

WorkspaceSelectPage::WorkspaceSelectPage(QWidget *parent) :
    QWizardPage(parent)
{
    newWorkspaceButton.setText("New workspace");
    connect(&newWorkspaceButton, SIGNAL(released()), this, SLOT(createNewWorkspace()));

    selectWorkspaceButton.setText("Select workspace");
    connect(&selectWorkspaceButton, SIGNAL(released()), this, SLOT(workspaceSelected()));

    layout.addWidget(new QLabel("Select a workspace to use"));
    layout.addWidget(&workspaceSelector);
    layout.addWidget(&selectWorkspaceButton);
    layout.addWidget(&newWorkspaceButton);

    setButtonText(QWizard::FinishButton, "OK");
    setButtonText(QWizard::CancelButton, "Cancel");

    setLayout(&layout);

    setTitle("Select Workspace");
}

void
WorkspaceSelectPage::setWorkspaceOptions(std::vector<QString> options) {
    for (auto &option : options) {
        workspaceSelector.addItem(option, QVariant(option));
    }
}

void
WorkspaceSelectPage::createNewWorkspace() {
    QFileDialog select(this);
    select.setFileMode(QFileDialog::Directory);
    select.setOption(QFileDialog::ShowDirsOnly, true);

    if (select.exec() == QDialog::Rejected) {
        return;
    }

    // get the list of files the user selected (should only be 1 directory)
    QStringList selectedFiles = select.selectedFiles();
    QMessageBox error;

    // make sure it is just 1 item
    if (selectedFiles.size() != 1) {
        error.critical(NULL, "Error", "you can only select one item");
        return;
    }

    QString selectedFile = selectedFiles[0];
    QFileInfo fileInfo(selectedFile);

    // it better exist, be a directory, be readable and writable
    if (!fileInfo.exists()) {
        error.critical(NULL, "Error", "It appears the file you chose doesnt exist");
        return;
    }

    if (!fileInfo.isDir()) {
        error.critical(NULL, "Error", "You must select a directory");
        return;
    }

    if (!fileInfo.isReadable()) {
        error.critical(NULL, "Error", "The directory you select must be readable");
        return;
    }

    if (!fileInfo.isWritable()) {
        error.critical(NULL, "Error", "The directory you select must be writable");
        return;
    }

    // make sure there isnt already a workspace in the directory the user selected,
    // if there is, ask them if they want to override the existing one or not
    if (Workspace::isWorkspace(selectedFile)) {
        QMessageBox::StandardButton reply =
                QMessageBox::question(this, "Workspace already exists",
                                      workspaceExistsMessage,
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    // attempt to create a workspace out of this directory
    QString errorMsg = Workspace::createWorkspace(selectedFile);

    if (errorMsg != "") {
        error.critical(NULL, "Error", errorMsg);
        return;
    }

    // add this workspace to our list of known workspaces
    MUSEGUIApplication::addWorkspaceEntry(selectedFile);

    // everything went well, close the wizard
    this->wizard()->accept();
}

void
WorkspaceSelectPage::workspaceSelected() {
    QString selectedFile = workspaceSelector.itemData(workspaceSelector.currentIndex()).toString();
    QMessageBox error;
    QFileInfo fileInfo(selectedFile);

    // the option the user selected needs to exist, be a directory, be readable
    // and writable
    if (!fileInfo.exists()) {
        error.critical(NULL, "Error", "It appears this workspace directory no longer exists");
        return;
    }

    if (!fileInfo.isDir()) {
        error.critical(NULL, "Error", "This workspace is not a directory");
        return;
    }

    if (!fileInfo.isReadable()) {
        error.critical(NULL, "Error", "This workspace is not readable");
        return;
    }

    if (!fileInfo.isWritable()) {
        error.critical(NULL, "Error", "This workspace is not writable");
        return;
    }

    // attempt to use this directory as the workspace
    QString errorMsg = Workspace::useWorkspace(selectedFile);

    if (errorMsg != "") {
        error.critical(NULL, "Error", errorMsg);
        return;
    }

    // everything went well, close the wizard
    this->wizard()->accept();
}

#endif
