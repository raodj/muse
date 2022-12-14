#ifndef SERVER_WIZARD_H
#define SERVER_WIZARD_H

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

#include "ServerTypePage.h"
#include "ServerInfoPage.h"
#include "ServerSummaryPage.h"

/**
 * @brief The ServerWizard class This class serves as the top-level class
 * for adding a new server entry to the work space.
 *
 * This top-level class merely creates the various pages and adds them to
 * the wizard. Each page performs a specific task required to create a
 * complete Server entry to be added to the workspace.
 */
class ServerWizard : public MUSEWizard {
public:
    ServerWizard(QFile& welcomeFile, QWidget* parent = 0);
    ServerWizard(QString& welcomeText, QWidget* parent = 0);

private:

    ServerTypePage serverTypePage;
    ServerInfoPage serverInfoPage;
    ServerSummaryPage serverSummaryPage;

    /**
     * @brief init Add all the necessary pages and signal connects needed
     * for this wizard.  This is just used so the constructors dont have
     * the same copy/pasted code
     */
    void init();

    /**
     * @brief passSessionToPages Sets the RemoteServerSession
     * instance variable pointer to the ServerTypePage and
     * ServerInfoPage so that they can use the same session to set up the
     * Server to be included in the workspace.
     */
    void passSessionToPages();

};

#endif
