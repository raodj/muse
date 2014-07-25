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
#include "LocalServerSession.h"
#include "Core.h"
#include "MUSEGUIApplication.h"
#include <QFile>
#include "Workspace.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>

#define SUCCESS_CODE 0
#define MAKE_JOB_FILE 0
#define SAVE_JOB_FILE 1
#define CONNECT_TO_SERVER 2
#define SEND_JOB_FILE 3
#define DELETE_LOCAL_JOB_FILE 4
#define ADD_JOB_TO_QUEUE 5
#define ADD_JOB_TO_WORKSPACE 6

SubmitPage::SubmitPage() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    prgDialog.setMinimum(0);
    prgDialog.setMaximum(6);
    prgDialog.setLabel(new QLabel("Submitting job"));
    mainLayout->addWidget(&prgDialog);
    setLayout(mainLayout);
    submitStep = 0;
    prgDialog.setCancelButton(0);
    safeToClose = false;
    setLayout(mainLayout);
}

SubmitPage::~SubmitPage() {

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
        ProjectList& projList = serverList.get(i).getProjectList();
        for (int j = 0; j < projList.size() && projectCount <= field("project").toInt();
             j++, projectCount++) {
            // Once we hit project n that matches the index of the project
            // field, print its info to the text edit.
            if (projectCount == field("project").toInt()) {
                proj = &projList.get(j);
            }
        }
    }
    QTimer::singleShot(500, this, SLOT(submitToServer()));

}

bool
SubmitPage::validatePage() {
    if (safeToClose) {
        // Temporary work around to prevent project from being
        // deleted from in-memory Workspace. This is only necessary
        // if a Project had been created prior to creating a Job
        // with the JobWizard.
        serverSession->disconnectFromServer();
        proj = NULL;
        server = NULL;
        delete job;
    }
    return safeToClose;
}

void
SubmitPage::connectedToServer(bool result) {
    if (result) {
        prgDialog.setValue(++submitStep);
        submitToServer();
    }
    else {
        prgDialog.setLabelText("Close the wizard and try again.");
        safeToClose = true;
    }
}

void
SubmitPage::submitToServer() {
    QString execDir = proj->getExecutablePath().left(proj->getExecutablePath().lastIndexOf("/"));
    if (submitStep == MAKE_JOB_FILE) {
        QString jobDescription = field("jobDescription").toString().isEmpty() ?
                    "None provided" : field("jobDescription").toString();
        job = new Job(proj->reserveJobId(), server->getID(),
                           -1, QDateTime::currentDateTime(), Job::Queued,
                      jobDescription);
        PBSJobFileCreator jobFile(job->getName(), field("estimatedRunTime").toInt(),
                                  field("memoryPerNode").toInt(), field("memoryUnits").toString(),
                                  field("nodes").toInt(), field("cpusPerNode").toInt(),
                                  field("arguments").toString(), execDir,
                                  proj->getExecutablePath().mid(proj->getExecutablePath().lastIndexOf("/") +1),
                                  field("wantsEmail").toBool());
        submitStep++;
        prgDialog.setValue(submitStep);
        // Save the file to the local machine before sending to server.
        jobFile.saveToFile(MUSEGUIApplication::getAppDirPath() + "/MUSEjob.job");
        submitStep++;
        prgDialog.setValue(submitStep);
        if (server->isRemote()) {
            serverSession = new RemoteServerSession(*server, NULL, "Adding a Job");
            RemoteServerSession* rss = dynamic_cast<RemoteServerSession*> (serverSession);
            rss->connectToServer();
            // Connect the signal to determine whether or not to proceed.
            connect(rss, SIGNAL(booleanResult(bool)),
                    this, SLOT(connectedToServer(bool)));
        }
        else {
            // This is a local server, proceed slightly differently.
            serverSession = new LocalServerSession(*server, NULL, "Adding a Job");
            submitStep++;
            prgDialog.setValue(submitStep);
            // Not exactly recursion since everything is excluded with else's.
            submitToServer();
        }
    }
    else if (submitStep == SEND_JOB_FILE) {
        // Send the job script to the server. This method should
        // change so that the filename isn't static.
        bool fileCopied = serverSession->copy(MUSEGUIApplication::getAppDirPath()
                                              + "/MUSEjob.job", execDir,
                                              job->getName() + ".job", 0666);
        if (fileCopied) {
            prgDialog.setValue(++submitStep);
            // We don't need the script file on the local computer...so delete it.
            QFile::remove(MUSEGUIApplication::getAppDirPath()
                          + "/MUSEjob.job");
            QString jobId, errMsg;
            // Did we succeed?
            if (serverSession->exec("qsub " + execDir + "/" + job->getName() + ".job -o"
                                    + proj->getOutputDirPath() + " -e " +
                                    proj->getOutputDirPath(),
                                    jobId, errMsg)  == SUCCESS_CODE) {
                job->setJobId(jobId.left(jobId.indexOf(".")).toLong());
                prgDialog.setValue(++submitStep);
                // Make the job workspace entry
                Workspace* ws = Workspace::get();
                // Add the entry to the workspace.
                ws->getJobList().addJob(*job);
                // Save the workspace
                ws->saveWorkspace();
                prgDialog.setValue(++submitStep);
                prgDialog.setLabelText("Submission Complete!");
                QMessageBox msg;
                msg.setText("Congratulations! The job was added successfully "\
                            "to the queue. It has been saved to the workspace. "\
                            "You may close this wizard.");
                msg.setDetailedText("The job id is: " + jobId);
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
                safeToClose = true;
            }
            else {
                // Exec failed.
                QMessageBox msg;
                msg.setText("Could not submit the job. Close the wizard and try again.");
                msg.setDetailedText(errMsg.trimmed());
                msg.setStandardButtons(QMessageBox::Ok);
                msg.exec();
                safeToClose = true;
            }
        }
        else {
            // Couldn't copy the script.
            QMessageBox msg;
            msg.setText("Could not send the script to the server. Please try running the wizard again.");
            msg.setStandardButtons(QMessageBox::Ok);
            msg.exec();
            safeToClose = true;
        }
    }

}

#endif
