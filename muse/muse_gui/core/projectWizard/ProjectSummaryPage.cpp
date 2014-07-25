#ifndef PROJECT_SUMMARY_PAGE_CPP
#define PROJECT_SUMMARY_PAGE_CPP

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

#include "ProjectSummaryPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include "Workspace.h"
#include <QMessageBox>
#include "RemoteServerSession.h"
#include "LocalServerSession.h"
#include "MUSEGUIApplication.h"

ProjectSummaryPage::ProjectSummaryPage(QWidget* parent) :
    QWizardPage (parent){
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("Project Name:"));
    mainLayout->addWidget(&projectName);
    mainLayout->addWidget(new QLabel("Source Files:"));
    mainLayout->addWidget(&sourceFiles);
    sourceFiles.setReadOnly(true);
    mainLayout->addWidget(new QLabel("Make File:"));

    mainLayout->addWidget(&makeFileDir);
    makeFileDir.setReadOnly(true);
    mainLayout->addWidget(new QLabel("Executable File:"));
    mainLayout->addWidget(&executableDir);
    executableDir.setReadOnly(true);

    mainLayout->addWidget(new QLabel("Output Directory:"));
    mainLayout->addWidget(&outputDir);

    outputDir.setReadOnly(true);

    setLayout(mainLayout);
}

void
ProjectSummaryPage::initializePage() {
    projectName.setText(field("projectName").toString());
    outputDir.setText(field("outputDirectory").toString());
    executableDir.setText(field("executable").toString());
    makeFileDir.setText(field("makeFile").toString());
    sourceFiles.clear();
    for (int i = 0; i < sourceFileList.size(); i++) {
        sourceFiles.append(sourceFileList.at(i));
    }
}

bool
ProjectSummaryPage::validatePage() {
    Project* pro = new Project(projectName.text(), makeFileDir.text(),
                     executableDir.text(), outputDir.text());
    // Since the list isn't a part of the constructor, we must set it
    // directly
    pro->setSourceFileList(sourceFileList);
    Server& server = serverSession->getServer();
    server.addProject(*pro);

    Workspace* ws = Workspace::get();
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
    delete pro;

    return true;
}

void
ProjectSummaryPage::receiveServerSession(ServerSession* ss) {
    serverSession = ss;
}

void
ProjectSummaryPage::receiveSourceList(QStringList list) {
    sourceFileList = list;
}

#endif
