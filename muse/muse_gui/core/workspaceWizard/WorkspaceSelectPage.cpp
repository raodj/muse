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

#include <QLabel>
#include <QFileDialog>
#include <QDialog>
#include <QStringList>
#include <QMessageBox>
#include <QFileInfo>

#include <iostream>

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
        option = option.trimmed();
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

    QStringList selectedFiles = select.selectedFiles();
    QMessageBox error;

    if (selectedFiles.size() != 1) {
        error.critical(NULL, "Error", "you can only select one item");
        return;
    }

    QString selectedFile = selectedFiles[0];
    QFileInfo fileInfo(selectedFile);

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

    QString errorMsg = Workspace::createWorkspace(selectedFile);

    if (errorMsg != "") {
        error.critical(NULL, "Error", errorMsg);
        return;
    }

    this->wizard()->accept();
}

void
WorkspaceSelectPage::workspaceSelected() {
    QString selectedFile = workspaceSelector.itemData(workspaceSelector.currentIndex()).toString();
    QMessageBox error;
    QFileInfo fileInfo(selectedFile);

    std::cout << "current: " << selectedFile.toStdString() << std::endl;
    std::cout << "index: " << workspaceSelector.currentIndex() << std::endl;

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

    QString errorMsg = Workspace::useWorkspace(selectedFile);

    if (errorMsg != "") {
        error.critical(NULL, "Error", errorMsg);
        return;
    }

    this->wizard()->accept();
}

#endif