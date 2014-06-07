#ifndef FIRST_RUN_WIZARD_CPP
#define FIRST_RUN_WIZARD_CPP

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

#include "FirstRunWizard.h"
#include "MUSEGUIApplication.h"
#include <QPalette>
#include <QDir>
#include <QFile>

FirstRunWizard::FirstRunWizard(MUSEGUIApplication& app, QWidget *parent) :
    MUSEWizard(parent), app(app) {
    setWindowTitle("First time setup");

    addPage(&welcomePage);
    addPage(&licensePage);
    addPage(&appDirPage);

    // stepListing = new SideWidget();
    setSideWidget(&stepListing);

    //Connect the signal of a page changing to update
    //the checklist showing the user's progress.
    //connect;

    //updateCheckList(1);
}

void
FirstRunWizard::createSideWidget() {
}

void
FirstRunWizard::initializePage(int id) {

    stepListing.applyCheckMarks(id + 1);
/*
    if (hasVisitedPage(0)) {
        welcomeStep->setPixmap(QPixmap(":/images/16x16/CheckMark.png"));
    }

    if (hasVisitedPage(1)) {
       licenseStep->setPixmap(QPixmap(":/images/16x16/CheckMark.png"));
    }

    if(hasVisitedPage(2))
        finish->setPixmap(QPixmap(":/images/16x16/CheckMark.png"));
*/

    page(id)->initializePage();

}

void
FirstRunWizard::cleanupPage(int id) {

    stepListing.applyCheckMarks(id);
    page(id)->cleanupPage();
}


void
FirstRunWizard::accept() {
    if (app.checkCreateAppDirectory(this)) {
        QDialog::accept();
    }
}

#endif
