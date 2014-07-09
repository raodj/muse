#ifndef PROJECT_DATA_PAGE_CPP
#define PROJECT_DATA_PAGE_CPP

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

#include "ProjectDataPage.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QFileDialog>
#include "CustomFileDialog.h"

ProjectDataPage::ProjectDataPage() {
    codeFileBrowse.setText("Browse");
    makeFileBrowse.setText("Browse");
    executableBrowse.setText("Browse");
    outputDirBrowse.setText("Browse");

    QVBoxLayout* mainLayout = new QVBoxLayout();
    // Add the labels and rows to the form layout
    addLineEditAndButtonToPage("Source Files:", &codeFilePath,
                               &codeFileBrowse, mainLayout);
    addLineEditAndButtonToPage("Make File:", &makeFilePath,
                               &makeFileBrowse, mainLayout);
    addLineEditAndButtonToPage("Executable:", &executabelPath,
                               &executableBrowse, mainLayout);
    addLineEditAndButtonToPage("Output Directory:", &outputDirPath,
                               &outputDirBrowse, mainLayout);
    // Set main layout as this page's layout.
    setLayout(mainLayout);
    // Set the line edits to be read only
    codeFilePath.setReadOnly(true);
    makeFilePath.setReadOnly(true);
    executabelPath.setReadOnly(true);
    outputDirPath.setReadOnly(true);

    setTitle("Project Information");
    setSubTitle("Set Details about the Project");
    registerFields();
    connectButtons();


}

void
ProjectDataPage::receiveServerSelection(RemoteServerSession* session) {
    this->session = session;
}

void
ProjectDataPage::connectButtons() {
    connect(&codeFileBrowse, SIGNAL(clicked()),
            this, SLOT(browseForSrcFiles()));
    connect(&makeFileBrowse, SIGNAL(clicked()),
            this, SLOT(browseForMakeFile()));
    connect(&executableBrowse, SIGNAL(clicked()),
            this, SLOT(browseForExecutable()));
    connect(&outputDirBrowse, SIGNAL(clicked()),
            this, SLOT(browseForOutputDir()));
}

void
ProjectDataPage::registerFields() {
    // The fields are all set to be required, so you can't accidentally
    // skip this page without selecting the files/folders
    registerField("codeFile*", &codeFilePath);
    registerField("makefile*", &makeFilePath);
    registerField("executable*", &executabelPath);
    registerField("outputDirectory*", &outputDirPath);
}

void
ProjectDataPage::addLineEditAndButtonToPage(QString label,
                                            QLineEdit* lineEdit,
                                            QPushButton* button,
                                            QVBoxLayout* layout){
    // The layout is a vertical alignment consisting of the label,
    // and then the line edit and button on the same row.
    QVBoxLayout* vertical = new QVBoxLayout();
    vertical->addWidget(new QLabel(label));

    QHBoxLayout* dataLayout = new QHBoxLayout();
    dataLayout->addWidget(lineEdit);
    dataLayout->addWidget(button);
    vertical->addLayout(dataLayout);

    layout->addLayout(vertical);


}

void
ProjectDataPage::browseForSrcFiles() {
//    codeFilePath.setText(QFileDialog::getOpenFileNames(
//                             NULL, "Select Directory of source files"));
}

void
ProjectDataPage::browseForMakeFile() {
//    makeFilePath.setText(QFileDialog::getOpenFileName(
//                             NULL, "Select MAKE File"));
}

void
ProjectDataPage::browseForExecutable() {
//    executabelPath.setText(QFileDialog::getOpenFileName(
//                               NULL, "Select Executable file"));
    CustomFileDialog cfd(session);
    executabelPath.setText(cfd.getOpenFileName());
}

void
ProjectDataPage::browseForOutputDir() {
//    outputDirPath.setText(QFileDialog::getExistingDirectory(
//                              NULL, "Select Directory for Output"));
}

#endif
