#ifndef PROJECT_SUMMARY_PAGE_H
#define PROJECT_SUMMARY_PAGE_H

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
#include <QTextEdit>
#include <QLineEdit>
#include "ServerSession.h"
#include <QStringList>

/**
 * @brief The ProjectSummaryPage class The final page of the
 * ProjectWizard that summarizes the file selections the user
 * made on the ProjectDataPage. Upon closing the wizard, this
 * page will add the Project to the workspace.
 */
class ProjectSummaryPage : public QWizardPage {
    Q_OBJECT
public:
    /**
     * @brief ProjectSummaryPage Simple constructor that creates the
     * layout for the page.
     * @param parent The widget this page belongs to.
     */
    ProjectSummaryPage(QWidget* parent = 0);

    /**
     * @brief initializePage An override of the QWizardPage method,
     * this method populates the four fields with the data from the
     * ProjectDataPage so that the user can review the information
     * and verify that it is correct before finishing the wizard.
     */
    void initializePage();

    /**
     * @brief validatePage An override of the QWizardPage method,
     * this method adds the project to the workspace before closing
     * the wizard.
     * @return For now, this only returns true, but depending on where
     * file verification is performed, this could change.
     */
    bool validatePage();

public slots:
    /**
     * @brief receiveServerSession Applies the server session
     * received via signal to this page's server session instance
     * variable.
     * @param ss The server session.
     */
    void receiveServerSession(ServerSession* ss);

    /**
     * @brief receiveSourceList Receives a copy of the list of source
     * files that are a part of the project.
     * @param list The list of file paths.
     */
    void receiveSourceList(QStringList list);

private:
    QTextEdit sourceFiles;
    QLineEdit makeFileDir, outputDir, executableDir, projectName;
    ServerSession* serverSession;
    QStringList sourceFileList;

};

#endif
