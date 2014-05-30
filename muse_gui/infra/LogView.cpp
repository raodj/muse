#ifndef LOG_VIEW_CPP
#define LOG_VIEW_CPP

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


#include "LogView.h"
#include <QFileDialog>
#include <QMessageBox>

LogView::LogView(QWidget *parent) : QWidget(parent){
    createToolBar();
}

void
LogView::createToolBar() {
    //logToolBar = new QToolBar();
    initializeFileNameDisplay();
    initializeActions();

    logToolBar.addWidget(new QLabel("Log filename: "));
    logToolBar.addWidget(&fileNameDisplay);
    logToolBar.addAction(changeLogFileName);
    logToolBar.addWidget(&saveToggleButton);

    connect(changeLogFileName, SIGNAL(triggered()),
            this, SLOT(selectNewLogFile()));
}

void
LogView::initializeFileNameDisplay() {
    fileNameDisplay.setText("<none set>");
    fileNameDisplay.setReadOnly(true);
}

void
LogView::initializeActions() {
    changeLogFileName = new QAction("Change log filename", this);
    changeLogFileName->setIcon(QIcon(":/images/16x16/ChangeLogFile.png"));
    // Setup the toggle button to start/stop saving logs.
    saveToggleButton.setIcon(QIcon(":/images/16x16/DontSaveLog.png"));
    saveToggleButton.setEnabled(false);
    saveToggleButton.setCheckable(true);
}

void
LogView::selectNewLogFile() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save File");
    if (!fileName.isEmpty()) {
        emit logFileNameChanged(fileName);
        saveToggleButton.setEnabled(true);
    }
}

void
LogView::updateSavePreference(bool saveEnabled) {
    saveToggleButton.setChecked(saveEnabled);
    if (saveEnabled) {
        saveToggleButton.setIcon(QIcon(":/images/16x16/SaveLog.png"));
    } else {
        saveToggleButton.setIcon(QIcon(":/images/16x16/DontSaveLog.png"));
    }
}

void
LogView::saveErrorNotification(const QString& errMsg) {
    QMessageBox msgBox;
    msgBox.setText("An error occured when saving the log file.");
    msgBox.setDetailedText(errMsg);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

#endif
