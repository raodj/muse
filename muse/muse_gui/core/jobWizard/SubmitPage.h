#ifndef SUBMIT_PAGE_H
#define SUBMIT_PAGE_H

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

#include <QWizardPage>
#include <QProgressDialog>
#include "Project.h"
#include "Server.h"
#include "ServerSession.h"
#include "Job.h"

/**
 * @brief The SubmitPage class Submits the job created throughout the
 * JobWizard to the designated server. This is the last page of the
 * JobWizard. The process cannot be stopped, and the user cannot backtrack
 * from this page.
 */
class SubmitPage : public QWizardPage {
    Q_OBJECT
public:
    /**
     * @brief SubmitPage Creates the layout of this page, which is only
     * a QProgresDialog without a cancel button at this time.
     */
    SubmitPage();
    ~SubmitPage();

    /**
     * @brief initializePage Sets the server and project variables
     * to the values as chosen by the user in earlier pages of the
     * JobWizard so that the needed information from these two entities
     * is easily accessible.
     */
    void initializePage();

    /**
     * @brief validatePage Runs the process of submitting the job to
     * the server. This description will be updated later once this method
     * has been fully implemented.
     * @return true if the page should advance, false otherwise.
     */
    bool validatePage();

private slots:
    /**
     * @brief connectedToServer Detects a signal from a RemoteServerSession
     * informing the program that the server has been connected to.
     * @param result True if the connection has been made, false otherwise.
     */
    void connectedToServer(bool result);

    /**
     * @brief submitToServer Submits the job to the server.
     */
    void submitToServer();

private:
    QProgressDialog prgDialog;
    QString projectName;
    Project* proj;
    int submitStep;
    bool safeToClose;

    ServerSession* serverSession;
    Server* server;
    Job* job;
};

#endif // SUBMITPAGE_H
