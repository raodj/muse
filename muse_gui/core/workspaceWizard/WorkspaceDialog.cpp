#ifndef WORKSPACE_WIZARD_CPP
#define WORKSPACE_WIZARD_CPP

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

#include "WorkspaceDialog.h"
#include "Workspace.h"
#include "MUSEGUIApplication.h"
#include "Version.h"

#include <QLabel>
#include <QList>
#include <QFileDialog>
#include <QMessageBox>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QSizePolicy>
#include <QTextEdit>

WorkspaceDialog::WorkspaceDialog(std::vector<QString> options, QWidget *parent) :
    QDialog(parent) {
    // Set a background image at the top to make the dialog box look pretty
    QPalette palette;
    QPixmap bgImg(":/images/logo/workspace_header.png");
    palette.setBrush(QPalette::Background, bgImg);
    setPalette(palette);
    // Add "known" workspace paths to the dialog box.
    for (auto &option : options) {
        workspaceSelector.addItem(option, QVariant(option));
    }
    workspaceSelector.setEditable(true);
    // Create the tabbed display widget with 2 tabs created using helpers
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->addTab(createWorkspaceTab(), "Workspace");
    tabWidget->addTab(createLicenseTab(), "License");

    // Organize the tab with a nice label on top that is right justified
    QLabel *logoLabel = new QLabel(this);
    logoLabel->setPixmap(QPixmap(":/images/logo/muse_white_logo.png"));
    QHBoxLayout *logoPanel = new QHBoxLayout();
    logoPanel->addStretch();
    logoPanel->addWidget(logoLabel);
    logoPanel->addStretch();
    // Create the top-level layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addSpacing(5);
    layout->addLayout(logoPanel);
    layout->addWidget(tabWidget);
    layout->setMargin(3);
    // Set the layout and control the maximum size to make background
    // and images look nice.
    setLayout(layout);
    setMaximumSize(800, 400);
}

QWidget*
WorkspaceDialog::createWorkspaceTab() {
    QWidget *widget = new QWidget();

    QPushButton *newWorkspaceButton = new QPushButton();
    newWorkspaceButton->setText("Browse");
    connect(newWorkspaceButton, SIGNAL(clicked()), this, SLOT(createNewWorkspace()));

    QPushButton *selectWorkspaceButton = new QPushButton();
    selectWorkspaceButton->setText("Ok");
    connect(selectWorkspaceButton, SIGNAL(clicked()), this, SLOT(workspaceSelected()));

    QPushButton *cancelButton = new QPushButton();
    cancelButton->setText("Cancel");
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *selectorLayout = new QHBoxLayout();
    selectorLayout->addWidget(&workspaceSelector, 1);
    selectorLayout->addWidget(newWorkspaceButton, 0);

    // Create label with lab logo and information.
    QLabel *labLogo = new QLabel();
    labLogo->setPixmap(QPixmap(":/images/logo/pc2lab_logo.png"));
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(labLogo);
    bottomLayout->addWidget(new QLabel(LAB_INFO_HTML), 1);
    bottomLayout->addSpacing(32);
    bottomLayout->addWidget(selectWorkspaceButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("Select a workspace to use or create a new one"));
    mainLayout->addLayout(selectorLayout);
    mainLayout->addSpacing(100);
    mainLayout->addLayout(bottomLayout);
    widget->setLayout(mainLayout);

    return widget;
}

QWidget*
WorkspaceDialog::createLicenseTab() {
    QWidget *widget = new QWidget();

    QFile license(":/resources/gpl.html");
    license.open(QIODevice::ReadOnly);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(new QTextEdit(license.readAll()));

    widget->setLayout(layout);

    return widget;
}

void
WorkspaceDialog::createNewWorkspace() {
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
                                      "The workspace you have selected already exists, would you like to " \
                                      "continue creating a new workspace in this directory?  All data from " \
                                      "the existing workspace will be lost.",
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
    accept();
}

void
WorkspaceDialog::workspaceSelected() {
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
    accept();
}

#endif
