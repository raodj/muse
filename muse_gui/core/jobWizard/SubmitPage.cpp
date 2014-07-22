#ifndef SUBMIT_PAGE_CPP
#define SUBMIT_PAGE_CPP

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

#include "SubmitPage.h"
#include "PBSJobFileCreator.h"
#include "RemoteServerSession.h"
#include "Core.h"
#include "MUSEGUIApplication.h"
#include <QFile>
#include "Workspace.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>

#define MAKE_JOB_FILE 0
#define SAVE_JOB_FILE 1
#define CONNECT_TO_SERVER 2
#define SEND_JOB_FILE 3
#define DELETE_LOCAL_JOB_FILE 4
#define ADD_JOB_TO_QUEUE 5

SubmitPage::SubmitPage() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    prgDialog.setMinimum(0);
    prgDialog.setMaximum(5);
    prgDialog.setLabel(new QLabel("Submitting job"));
    mainLayout->addWidget(&prgDialog);
    setLayout(mainLayout);
    submitStep = 0;
    prgDialog.setCancelButton(0);

    setLayout(mainLayout);
}

void
SubmitPage::initializePage() {
    Workspace* ws = Workspace::get();
    ServerList& serverList = ws->getServerList();
    server = &serverList.get(field("server").toInt());
    // Used to compare the number of projects found and the index
    // of the project selected in the QComboBox on a previous page.
    int projectCount = 0;
    for (int i = 0; i < serverList.size(); i++) {
        ProjectList projList = serverList.get(i).getProjectList();
        for (int j = 0; j < projList.size() && projectCount <= field("project").toInt();
             j++, projectCount++) {
            // Once we hit project n that matches the index of the project
            // field, print its info to the text edit.
            if (projectCount == field("project").toInt()) {
                proj = &projList.get(j);
            }
        }
    }
    QTimer::singleShot(500, this->wizard(), SLOT(next()));

}

bool
SubmitPage::validatePage() {

    if (submitStep == MAKE_JOB_FILE) {
        PBSJobFileCreator jobFile(proj->getName(), field("estimatedRunTime").toInt(),
                                  field("memoryPerNode").toInt(),
                                  field("nodes").toInt(), field("cpusPerNode").toInt(),
                                  field("arguments").toString(), proj->getExecutablePath().left(proj->getExecutablePath().lastIndexOf("/")),
                                  proj->getExecutablePath().mid(proj->getExecutablePath().lastIndexOf("/") +1),
                                  true);
        submitStep++;
        prgDialog.setValue(submitStep);
        jobFile.saveToFile(MUSEGUIApplication::getAppDirPath() + "/MUSEjob.job");
        submitStep++;
        prgDialog.setValue(submitStep);
//        if (server->isRemote()) {
        serverSession = new RemoteServerSession(*server, NULL, "Adding a Job");
//        }
//        else {
//            serverSession = new LocalServerSession(*server, NULL, "Adding a Job");
//            submitStep++;
//        }
        serverSession->connectToServer();
        return false;
    }

    return false;
}

void
SubmitPage::connectedToServer(bool result) {
    if (result) {
        prgDialog.setValue(++submitStep);
    }
}

#endif
