#ifndef FIRST_RUN_WIZARD_H
#define FIRST_RUN_WIZARD_H

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
#include "MUSEWizard.h"
#include "WelcomePage.h"
#include "LicensePage.h"
#include "AppDirPage.h"
#include "MUSEGUIApplication.h"

/**
 * @brief The FirstRunWizard class The class that is run to set up
 * the workspace for the user.
 *
 * This wizard only runs the first time the MUSE GUI is run to validate
 * EULA and create the top-level application data directory
 * for storing default set of configuration and meta data files.
 *
 * FirstRunWizard is an extension of the MUSEWizard class.
 */
class FirstRunWizard : public MUSEWizard {
public:
    FirstRunWizard(MUSEGUIApplication& app, QWidget* parent = 0);

    /**
     * @brief accept The MUSE-specific implementation of the accept()
     * signal, which is called when the user successfully completes
     * the QWizard. The program will create the default application
     * directory before closing the dialog.
     */
    void accept();



private:
    /**
     * @brief createSideWidget Initializes the side widget to display
     * a checklist of all of the steps in the FirstRunWizard.
     */
    void createSideWidget();

    /**
     * @brief app The top-level MUSE GUI application from where this
     * wizard was launched.  This reference is essentially used to
     * invoke the MUSEGUIApplication::checkCreateAppDirectory() method
     * to create the top-level application directory and various other
     * necessary files the first time MUSE-GUI is run by an user.
     */
    MUSEGUIApplication& app;


    WelcomePage welcomePage;
    LicensePage licensePage;
    AppDirPage  appDirPage;
};

#endif
