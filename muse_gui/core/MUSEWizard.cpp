#ifndef MUSEWIZARD_CPP
#define MUSEWIZARD_CPP
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
#include <QHBoxLayout>

MUSEWizard::MUSEWizard(QWidget *parent) : QWizard(parent) {
    setPixmap(QWizard::BannerPixmap,
              QPixmap(":/images/logo/bannerImg.png"));
    setPixmap(QWizard::BackgroundPixmap,
              QPixmap(":/images/logo/columnImg.png"));

    mainLayout = new QVBoxLayout();
    sideBar.setLayout(mainLayout);
    setSideWidget(&sideBar);

#ifndef Q_OS_MAC
    setWizardStyle(QWizard::ModernStyle);
#endif
}

int
MUSEWizard::addPage(QWizardPage *page, const QString& stepName,
                    const bool lastPage) {
    QLabel* const box = new QLabel();
    box->setPixmap(QPixmap(":/images/16x16/Box.png"));
    QLabel* const step = new QLabel(stepName);
    // Organize and add checkbox label and step-name to wizard.
    QHBoxLayout* row = new QHBoxLayout();
    row->addWidget(box);
    row->addWidget(step, 1);
    // The wizard to the wizard.
    mainLayout->addLayout(row);
    // Once all pages are in place, add a stretcher to make steps look nice
    if (lastPage) {
        mainLayout->addStretch(1);
    }
    // Save labels for updates as user navigates through wizard page.
    steps.append(box);
    steps.append(step);
    // Let base clas handle actual addition of the page.
    return QWizard::addPage(page);
}


void
MUSEWizard::initializePage(int id) {
    applyCheckMarks(id + 1);
    page(id)->initializePage();
}

void
MUSEWizard::cleanupPage(int id) {
    applyCheckMarks(id);
    page(id)->cleanupPage();
}

void
MUSEWizard::applyCheckMarks(const int pageId) {
    static const QPixmap box(":/images/16x16/Box.png");
    static const QPixmap checkBox(":/images/16x16/CheckedBox.png");

    for(int i = 0; i < steps.size(); i += 2) {
        steps.at(i)->setPixmap((pageId > i / 2) ? checkBox : box);
    }
}

#endif
