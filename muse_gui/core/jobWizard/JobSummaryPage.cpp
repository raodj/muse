#ifndef JOB_SUMMARY_PAGE_CPP
#define JOB_SUMMARY_PAGE_CPP
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

#include "JobSummaryPage.h"
#include "Core.h"
#include <QVBoxLayout>
#include <QLabel>
#include "Workspace.h"

JobSummaryPage::JobSummaryPage() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("You have provided the following information "\
                                     "regarding the job to be scheduled.\nPlease "\
                                     "verify the information before submitting the job."));
    mainLayout->addWidget(&summaryDisplay);
    summaryDisplay.setReadOnly(true);
    warningMessage.setIcon(QMessageBox::Warning);
    warningMessage.setText("Once you click the 'Next' button, you cannot backtrack to this page.");
    warningMessage.setStandardButtons(0);
    mainLayout->addWidget(&warningMessage);
    setLayout(mainLayout);
    setTitle("Job Summary");
    setCommitPage(true);
}

void
JobSummaryPage::initializePage() {
    summaryDisplay.append("SUMMARY OF JOB");
    summaryDisplay.append("Job Information:");
    // For convenience and readability.
    QString indent = "\t\t";
    Workspace* ws = Workspace::get();
    ServerList& serverList = ws->getServerList();
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
                summaryDisplay.append(indent +"Project: "
                                      + projList.get(j).getName());
            }
        }
    }
    summaryDisplay.append(indent + "Job Description: "+
                          field("jobDescription").toString());
    summaryDisplay.append("Server Information:");
    summaryDisplay.append(indent + serverList.get(field("server").toInt()).getName());
    summaryDisplay.append(indent + "Nodes: " + QString::number(
                              field("nodes").toInt()));
    summaryDisplay.append(indent + "CPUs per Node: " + QString::number(
                              field("cpusPerNode").toInt()));
    summaryDisplay.append(indent + "Memory per Node (MB): " +
                          QString::number(field("memoryPerNode").toInt()));
    summaryDisplay.append(indent + "Est. Run Time (Hrs): " +
                          QString::number(field("estimatedRunTime").toInt()));
    summaryDisplay.append("Executable File Path: " + field("executablePath")
                          .toString());
}

#endif
