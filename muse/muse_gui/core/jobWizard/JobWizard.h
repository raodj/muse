#ifndef JOB_WIZARD_H
#define JOB_WIZARD_H

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

#include "MUSEWizard.h"
#include "JobInformationPage.h"
#include "ServerSetupPage.h"
#include "JobSummaryPage.h"
#include "SubmitPage.h"

/**
 * @brief The JobWizard class A wizard to submit a job to a Server running
 * MUSE. This class will first verify that the user has a project and server
 * in the workspace before presenting itself. Then it will create the wizard
 * by adding the necessary pages, and then connect any signals between pages
 * as needed.
 */
class JobWizard : public MUSEWizard {
    Q_OBJECT
public:
    /**
     * @brief JobWizard First checks the workspace for a Server and Project
     * to ensure that the user has the necessary information to complete
     * this wizard. If it doesn't, a warning message will display and
     * the wizard will not appear. If the workspace has the information
     * required, then the pages will get added to the server.
     * @param file The file pointing the text for the overview page.
     */
    JobWizard(QFile& file);

public slots:
    /**
     * @brief exec First checks whether or not a project exists and that a
     * server is in the workspace. If either of these items is missing,
     * the wizard will fail to appear, instead displaying a warning message.
     * If everything checks out, the parent class exec() is called to run
     * the wizard.
     * @return The exit code.
     */
    int exec();

private:
    JobInformationPage jobInformationPage;
    ServerSetupPage serverSetupPage;
    JobSummaryPage summaryPage;
    SubmitPage submitPage;
};

#endif // JOBWIZARD_H
