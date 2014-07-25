#ifndef JOB_WIZARD_CPP
#define JOB_WIZARD_CPP

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

#include "Core.h"
#include "JobWizard.h"
#include "Workspace.h"
#include <QMessageBox>

JobWizard::JobWizard(QFile& file) : MUSEWizard(file) {
    addPage(&jobInformationPage, "Job Information");
    addPage(&serverSetupPage, "Server Setup");
    addPage(&summaryPage, "Summary Page");
    addPage(&submitPage, "Job Submission", true);
}

int
JobWizard::exec() {
    if (!checkRequirements()) {
        // Something missing, bail out.
        return 0;
    }
    // Everything checks out, run the wizard.
    return QWizard::exec();
}

bool
JobWizard::checkRequirements() {
    ServerList servers = Workspace::get()->getServerList();
    // Check that a server exists
    if (servers.size() == 0) {
        QMessageBox msg;
        msg.setIcon(QMessageBox::Critical);
        msg.setText("No servers were found in the workspace. A server" \
                    " is required to create a job. Please create a server " \
                     "(and then a project) using the appropriate wizards."\
                    " Then run this wizard again.");
        msg.exec();
        return false;
    }
    // If a server exists, check for a Project
    else {
        bool projectFound = false;
        for (int i = 0; i < servers.size() && !projectFound; i ++) {
            if (servers.get(i).getProjectList().size() > 0) {
                projectFound = true;
            }
        }
        if (!projectFound) {
            QMessageBox msg;
            msg.setIcon(QMessageBox::Critical);
            msg.setText("No projects were found in the workspace. A project" \
                        " is required to create a job. Please create a project " \
                         "using the project wizard." \
                        " Then run this wizard again.");
            msg.exec();
            return false;
        }

    }
    return true;
}

#endif
