#ifndef FIRSTRUNWIZARD_CPP
#define FIRSTRUNWIZARD_CPP
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

#include "MUSEApplicationDirectory.h"

#include <QPalette>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QList>


FirstRunWizard::FirstRunWizard(QWidget *parent)
    : MUSEWizard(parent) {

    setWindowTitle("First time setup");
    //setWizardStyle(QWizard::ModernStyle);

    welcomePage = new WelcomePage();
    licensePage = new LicensePage();
    appDirPage = new AppDirPage();

    addPage(welcomePage);
    addPage(licensePage);
    addPage(appDirPage);

    createSideWidget();

    setSideWidget(stepListing);

    //Connect the signal of a page changing to update
    //the checklist showing the user's progress.
    //connect;

    //updateCheckList(1);
}

void
FirstRunWizard::createSideWidget() {

    stepListing = new QWidget();



    welcomeStep = new QCheckBox();
    QPalette p = welcomeStep->palette();
    p.setColor(QPalette::Active, QPalette::WindowText, Qt::white);
    welcomeStep->setPalette(p);
    welcomeStep->setText("Welcome");
    welcomeStep->setEnabled(false);

    licenseStep = new QCheckBox();
    licenseStep->setText("License Agreement");
    licenseStep->setPalette(p);
    licenseStep->setEnabled(false);


    finish = new QCheckBox();
    finish->setPalette(p);
    finish->setText("Finish");
    finish->setEnabled(false);

    sideLayout = new QVBoxLayout();
    sideLayout->addWidget(welcomeStep);
    sideLayout->addWidget(licenseStep);
    sideLayout->addWidget(finish);

    stepListing->setLayout(sideLayout);
}

void
FirstRunWizard::initializePage(int id) {


    if (hasVisitedPage(0)) {
        welcomeStep->setChecked(true);
    }

    if (hasVisitedPage(1)) {
       licenseStep->setChecked(true);
    }

    if(hasVisitedPage(2))
        finish->setChecked(true);


    page(id)->initializePage();

}

void
FirstRunWizard::cleanupPage(int id) {
    page(id)->cleanupPage();
    if (id == 0) {
        welcomeStep->setChecked(false);
    }

    if (id == 1) {
       licenseStep->setChecked(false);
    }

    if(id == 2)
        finish->setChecked(false);
}


void
FirstRunWizard::accept() {

    QDir workspaceDir(MUSEApplicationDirectory::getAppDirPath());

    workspaceDir.mkdir(MUSEApplicationDirectory::getAppDirPath());

    QFile workspace(MUSEApplicationDirectory::getKnownHostsPath());
    workspace.open(QFile::ReadWrite);

    workspace.close();

    QDialog::accept();
}


#endif
