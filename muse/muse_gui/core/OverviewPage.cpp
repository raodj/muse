#ifndef OVERVIEW_PAGE_CPP
#define OVERVIEW_PAGE_CPP

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

#include "OverviewPage.h"
#include <QTextStream>
#include <QVBoxLayout>

OverviewPage::OverviewPage(const QFile &file, QWidget* parent) :
    QWizardPage(parent) {
    // User can't edit the text in this view port.
    overviewText.setReadOnly(true);
    // Load the text from the html file. Need temporary ioFile
    // to call non-const methods.
    QFile ioFile(file.fileName());
    if (ioFile.open(QFile::ReadOnly)) {
        QTextStream input(&ioFile);
        overviewText.setHtml(input.readAll());
        ioFile.close();
    }
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 2, 0, 2);
    mainLayout->addWidget(&overviewText);
    setLayout(mainLayout);

    // Fixing for Mac display currently.
    setTitle("Overview");
    setSubTitle("Overview of tasks in this wizard");
}

OverviewPage::OverviewPage(const QString& text, QWidget *parent) :
    QWizardPage(parent)  {
    // User can't edit the text in this view port.
    overviewText.setReadOnly(true);
    overviewText.setText(text);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(&overviewText);
    setLayout(mainLayout);

    setTitle("Overview");
    setSubTitle("Overview of tasks in this wizard");
}

void
OverviewPage::setText(const QString& text) {
    overviewText.setText(text);
}

#endif
