#ifndef JOB_INFORMATION_PAGE_CPP
#define JOB_INFORMATION_PAGE_CPP

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

#include "JobInformationPage.h"
#include "Workspace.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

JobInformationPage::JobInformationPage() {
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("Select Project"));
    mainLayout->addWidget(&projectSelector);
    mainLayout->addWidget(new QLabel("Command line arguments for this job"));
    mainLayout->addWidget(&argumentsField);
    mainLayout->addWidget(new QLabel("Description for this job: (for your reference)"));
    mainLayout->addWidget(&descriptionField);
    setLayout(mainLayout);

    registerField("project", &projectSelector);
    registerField("jobDescription", &descriptionField, "plainText");
    registerField("arguments", &argumentsField, "plainText");

    Workspace* ws = Workspace::get();
    ServerList& serverList = ws->getServerList();

    for (int i = 0; i < serverList.size(); i++) {
        ProjectList projList = serverList.get(i).getProjectList();
        for (int j = 0; j < projList.size(); j++) {
            projectSelector.addItem(projList.get(j).getName());
        }
    }
    setTitle("Job Information");
}

#endif
